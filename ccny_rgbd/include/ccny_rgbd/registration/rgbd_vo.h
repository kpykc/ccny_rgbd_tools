#ifndef CCNY_RGBD_RGBD_VO_H
#define CCNY_RGBD_RGBD_VO_H

#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/image_encodings.h>
#include <std_msgs/Time.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <pcl/point_types.h>
#include <pcl/io/io.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/correspondence_estimation.h>
#include <pcl/registration/correspondence_rejection_sample_consensus.h>
#include <pcl_ros/point_cloud.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <image_geometry/pinhole_camera_model.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

#include "ccny_rgbd/rgbd_util.h"
#include "ccny_rgbd/structures/rgbd_frame.h"
#include "ccny_rgbd/features/feature_detector.h"
#include "ccny_rgbd/features/klt_detector.h"
#include "ccny_rgbd/features/orb_detector.h"
#include "ccny_rgbd/features/surf_detector.h"
#include "ccny_rgbd/features/sift_gpu_detector.h"
#include "ccny_rgbd/features/gft_detector.h"
#include "ccny_rgbd/features/fast_detector.h"
#include "ccny_rgbd/registration/motion_estimation.h"
#include "ccny_rgbd/registration/motion_estimation_icp.h"
#include "ccny_rgbd/registration/motion_estimation_icp_model.h"
#include "ccny_rgbd/registration/motion_estimation_icp_prob_model.h"
#include "ccny_rgbd/registration/motion_estimation_icp_loop.h"
#include "ccny_rgbd/registration/motion_estimation_icp_particles.h"
#include "ccny_rgbd/registration/motion_estimation_ransac.h"
#include "ccny_rgbd/registration/motion_estimation_ransac_model.h"

#include "ccny_rgbd/loop/keyframe_generator.h"
#include "ccny_rgbd/loop/loop_solver.h"
#include "ccny_rgbd/loop/loop_solver_sba.h"
#include "ccny_rgbd/loop/loop_solver_gicp.h"

namespace ccny_rgbd
{

using namespace message_filters::sync_policies;

class RGBDVO
{
  typedef image_transport::SubscriberFilter ImageSubFilter;
  typedef message_filters::Subscriber<sensor_msgs::CameraInfo> CameraInfoSubFilter;
  typedef ApproximateTime<sensor_msgs::Image, sensor_msgs::Image, sensor_msgs::CameraInfo> SyncPolicy;
  typedef message_filters::Synchronizer<SyncPolicy> Synchronizer;
  typedef nav_msgs::Odometry OdomMsg;

  public:

    RGBDVO(ros::NodeHandle nh, ros::NodeHandle nh_private);
    virtual ~RGBDVO();

  private:

    // **** ROS-related

    ros::NodeHandle nh_;
    ros::NodeHandle nh_private_;
    tf::TransformListener tf_listener_;
    tf::TransformBroadcaster tf_broadcaster_;
    ros::Publisher odom_publisher_;

    boost::shared_ptr<image_transport::ImageTransport> rgb_it_;
    boost::shared_ptr<image_transport::ImageTransport> depth_it_;
    boost::shared_ptr<Synchronizer> sync_;
       
    ImageSubFilter      sub_depth_;
    ImageSubFilter      sub_rgb_;
    CameraInfoSubFilter sub_info_;

    // **** parameters 

    std::string fixed_frame_; 
    std::string base_frame_;

    std::string detector_type_;
    std::string reg_type_;

    // **** variables

    boost::mutex mutex_;
    bool initialized_;
    int  frame_count_;

    tf::Transform b2c_;
    tf::Transform f2b_;

    FeatureDetector * feature_detector_;

    MotionEstimation * motion_estimation_;

    KeyframeGenerator * keyframe_generator_;
  
    // **** private functions

    void imageCb(const sensor_msgs::ImageConstPtr& depth_msg,
                 const sensor_msgs::ImageConstPtr& rgb_msg,
                 const sensor_msgs::CameraInfoConstPtr& info_msg);

    void initParams();

    void publishTf(const std_msgs::Header& header);

    bool getBaseToCameraTf(const std_msgs::Header& header);
};

} //namespace ccny_rgbd

#endif // CCNY_RGBD_VO_H
