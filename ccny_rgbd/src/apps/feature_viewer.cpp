/**
 *  @file feature_viewer.cpp
 *  @author Ivan Dryanovski <ivan.dryanovski@gmail.com>
 * 
 *  @section LICENSE
 * 
 *  Copyright (C) 2013, City University of New York
 *  CCNY Robotics Lab <http://robotics.ccny.cuny.edu>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ccny_rgbd/apps/feature_viewer.h"

namespace ccny_rgbd {
  
FeatureViewer::FeatureViewer(
  const ros::NodeHandle& nh, 
  const ros::NodeHandle& nh_private):
  nh_(nh), 
  nh_private_(nh_private),
  frame_count_(0)
{
  ROS_INFO("Starting RGBD Feature Viewer");
  
  // **** initialize ROS parameters
  
  initParams();
  
  mutex_.lock();
  resetDetector();
  mutex_.unlock();

  // dynamic reconfigure
  FeatureDetectorConfigServer::CallbackType f = boost::bind(
    &FeatureViewer::reconfigCallback, this, _1, _2);
  config_server_.setCallback(f);
  
  // **** publishers
  
  cloud_publisher_ = nh_.advertise<PointCloudFeature>(
    "feature_cloud", 1);
  covariances_publisher_ = nh_.advertise<visualization_msgs::Marker>(
    "feature_covariances", 1);
  
  // **** subscribers
  
  ImageTransport rgb_it(nh_);
  ImageTransport depth_it(nh_);

  sub_rgb_.subscribe(rgb_it,     "/rgbd/rgb",   queue_size_);
  sub_depth_.subscribe(depth_it, "/rgbd/depth", queue_size_);
  sub_info_.subscribe(nh_,       "/rgbd/info",  queue_size_);

  // Synchronize inputs.
  sync_.reset(new RGBDSynchronizer3(
                RGBDSyncPolicy3(queue_size_), sub_rgb_, sub_depth_, sub_info_));
  
  sync_->registerCallback(boost::bind(&FeatureViewer::RGBDCallback, this, _1, _2, _3));  
}

FeatureViewer::~FeatureViewer()
{
  ROS_INFO("Destroying RGBD Feature Viewer"); 

  //delete feature_detector_;
}

void FeatureViewer::initParams()
{ 
  if (!nh_private_.getParam ("queue_size", queue_size_))
    queue_size_ = 5;
  if (!nh_private_.getParam ("feature/detector_type", detector_type_))
    detector_type_ = "GFT";
  if (!nh_private_.getParam ("feature/show_keypoints", show_keypoints_))
    show_keypoints_ = false;
  if (!nh_private_.getParam ("feature/publish_cloud", publish_cloud_))
    publish_cloud_ = false;
  if (!nh_private_.getParam ("feature/publish_covariances", publish_covariances_))
    publish_covariances_ = false;
}

void FeatureViewer::resetDetector()
{  
  if (detector_type_ == "ORB")
  { 
    ROS_INFO("Creating ORB detector");
    feature_detector_.reset(new OrbDetector(nh_, nh_private_));
  }
  else if (detector_type_ == "SURF")
  {
    ROS_INFO("Creating ORB detector");
    feature_detector_.reset(new SurfDetector(nh_, nh_private_));
  }
  else if (detector_type_ == "GFT")
  {
    ROS_INFO("Creating GFT detector");
    feature_detector_.reset(new GftDetector(nh_, nh_private_));
  }
  else if (detector_type_ == "STAR")
  {
    ROS_INFO("Creating STAR detector");
    feature_detector_.reset(new StarDetector(nh_, nh_private_));
  }
  else
  {
    ROS_FATAL("%s is not a valid detector type! Using GFT", detector_type_.c_str());
    feature_detector_.reset(new GftDetector(nh_, nh_private_));
  }
}

void FeatureViewer::RGBDCallback(
  const ImageMsg::ConstPtr& rgb_msg,
  const ImageMsg::ConstPtr& depth_msg,
  const CameraInfoMsg::ConstPtr& info_msg)
{
  mutex_.lock();
  
  ros::WallTime start = ros::WallTime::now();

  // create frame
  RGBDFrame frame(rgb_msg, depth_msg, info_msg);

  // find features
  feature_detector_->findFeatures(frame);
 
  ros::WallTime end = ros::WallTime::now();
  
  // visualize 
  
  if (show_keypoints_) showKeypointImage(frame);
  if (publish_cloud_) publishFeatureCloud(frame);
  if (publish_covariances_) publishFeatureCovariances(frame);
  
  // print diagnostics

  int n_features = frame.keypoints.size();
  int n_valid_features = frame.n_valid_keypoints;

  double d_total = 1000.0 * (end - start).toSec();

  printf("[FV %d] %s[%d][%d]: TOTAL %3.1f\n",
    frame_count_, detector_type_.c_str(), n_features, n_valid_features, d_total);

  frame_count_++;
  
  mutex_.unlock();
}

void FeatureViewer::showKeypointImage(RGBDFrame& frame)
{
  cv::namedWindow("Keypoints", CV_WINDOW_NORMAL);
  cv::Mat kp_img(frame.rgb_img.size(), CV_8UC1);
  cv::drawKeypoints(frame.rgb_img, frame.keypoints, kp_img);
  cv::imshow("Keypoints", kp_img);
  cv::waitKey(1);
}

void FeatureViewer::publishFeatureCloud(RGBDFrame& frame)
{
  PointCloudFeature cloud;
  cloud.header = frame.header;
  frame.constructFeaturePointCloud(cloud);   
  cloud_publisher_.publish(cloud);
}

void FeatureViewer::publishFeatureCovariances(RGBDFrame& frame)
{
  // create markers
  visualization_msgs::Marker marker;
  marker.header = frame.header;
  marker.type = visualization_msgs::Marker::LINE_LIST;
  marker.color.r = 1.0;
  marker.color.g = 1.0;
  marker.color.b = 1.0;
  marker.color.a = 1.0;
  marker.scale.x = 0.0025;
  marker.action = visualization_msgs::Marker::ADD;
  marker.ns = "covariances";
  marker.id = 0;
  marker.lifetime = ros::Duration();

  for (unsigned int kp_idx = 0; kp_idx < frame.keypoints.size(); ++kp_idx)
  {
    if (!frame.kp_valid[kp_idx]) continue;
    
    const Vector3f& kp_mean = frame.kp_means[kp_idx];
    const Matrix3f& kp_cov  = frame.kp_covariances[kp_idx];

    // transform Eigen to OpenCV matrices
    cv::Mat m(3, 1, CV_64F);
    for (int j = 0; j < 3; ++j)
      m.at<double>(j, 0) = kp_mean(j, 0);

    cv::Mat cov(3, 3, CV_64F);
    for (int j = 0; j < 3; ++j)
    for (int i = 0; i < 3; ++i)
      cov.at<double>(j, i) = kp_cov(j, i);

    // compute eigenvectors
    cv::Mat evl(1, 3, CV_64F);
    cv::Mat evt(3, 3, CV_64F);
    cv::eigen(cov, evl, evt);

    double mx = m.at<double>(0,0);
    double my = m.at<double>(1,0);
    double mz = m.at<double>(2,0);

    for (int e = 0; e < 3; ++e)
    {
      geometry_msgs::Point a;
      geometry_msgs::Point b;

      double sigma = sqrt(evl.at<double>(0,e));
      double scale = sigma * 3.0;
      tf::Vector3 evt_tf(evt.at<double>(e,0), 
                         evt.at<double>(e,1), 
                         evt.at<double>(e,2));
    
      a.x = mx + evt_tf.getX() * scale;
      a.y = my + evt_tf.getY() * scale;
      a.z = mz + evt_tf.getZ() * scale;
   
      b.x = mx - evt_tf.getX() * scale;
      b.y = my - evt_tf.getY() * scale;
      b.z = mz - evt_tf.getZ() * scale;

      marker.points.push_back(a);
      marker.points.push_back(b);
    }
  }

  covariances_publisher_.publish(marker);
}

void FeatureViewer::reconfigCallback(FeatureDetectorConfig& config, uint32_t level)
{ 
  mutex_.lock();
  std::string old_detector_type = detector_type_;
  detector_type_ = config.detector_type;
  
  if(old_detector_type != detector_type_)
    resetDetector();   
  
  feature_detector_->setSmooth(config.smooth);
  feature_detector_->setMaxRange(config.max_range);
  feature_detector_->setMaxStDev(config.max_stdev);
  
  publish_cloud_ = config.publish_cloud;
  publish_covariances_ = config.publish_covariances;
  show_keypoints_ = config.show_keypoints;
  
  mutex_.unlock();
}

} //namespace ccny_rgbd