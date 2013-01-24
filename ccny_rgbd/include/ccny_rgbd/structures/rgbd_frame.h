/**
 *  @file rgbd_frame.h
 *  @author Ivan Dryanovski <ivan.dryanovski@gmail.comm>
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

#ifndef CCNY_RGBD_RGBD_FRAME_H
#define CCNY_RGBD_RGBD_FRAME_H

#include <vector>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/image_encodings.h>
#include <image_geometry/pinhole_camera_model.h>
#include <boost/filesystem.hpp>
#include <pcl/io/pcd_io.h>

#include "ccny_rgbd/types.h"
#include "ccny_rgbd/rgbd_util.h"

namespace ccny_rgbd {

/** @brief Auxiliarry class that holds together rgb and depth images.
 * 
 * The class also holds the detected keypoints and their 3D distributions
 * Keypoints, descriptos, and kp_* are index the same way and may include 
 * invalid points. An invalid keypoint occurs when:
 *  - no z data 
 *  - z > threshold
 *  - var_z > threshold
 */
class RGBDFrame
{
  public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /** @brief Default (empty) constructor.
     */
    RGBDFrame();

    /** @brief Constructor from ROS messages
     * @param rgb_msg 8UC3 ROS image message
     * @param depth_msg 16UC1 ROS depth message (in mm, 0 = invalid data)
     * @param info_msg ROS camera info message, assumed no distortion, applies to both images
     */
    RGBDFrame(const ImageMsg::ConstPtr& rgb_msg,
              const ImageMsg::ConstPtr& depth_msg,
              const CameraInfoMsg::ConstPtr& info_msg);
        
    std_msgs::Header header; ///< Header taken from rgb_msg
    cv::Mat rgb_img;         ///< RGB image (8UC3)
    cv::Mat depth_img;       ///< Depth image in mm (16UC1). 0 = invalid data

    /** @brief The intrinsic matrix which applies to both images. 
     * 
     * It's assumed that the images are already
     * rectified and in the same camera frame(RGB)
     */
    image_geometry::PinholeCameraModel model;
    
    KeypointVector keypoints;         ///< 2D keypoint locations
    cv::Mat        descriptors;       ///< Feature descriptor vectors

    BoolVector     kp_valid;          ///< Is the z data valid?
    Vector3fVector kp_means;          ///< 1x3 mat of 3D locations
    Matrix3fVector kp_covariances;    ///< 3x3 mat of covariances

    int n_valid_keypoints;            ///< How many keypoints have usable 3D data

    /** @brief Computes the 3D means and covariances for all the detected keypoints.
     *  
     * Some features will be marked as invalid.
     * @todo do we want default values? 
     * @param max_z [m] features with z bigger than this will be marked as invalid
     * @param max_stdev_z [m] features with std_dev(z) bigger than this 
     *        will be marked as invalid
     */
    void computeDistributions(
      double max_z = 5.5,
      double max_stdev_z = 0.03);    

    /** @brief Computes the 3D means and covariances for all the valid detected keypoints.
     * @param cloud Reference to the point cloud to be constructed
     */  
    void constructFeaturePointCloud(PointCloudFeature& cloud);

    /** @brief Saves the RGBD frame to disk. 
    * 
    * Saves the RGB and depth images as png, and the header and intrinsic matrix
    * as .yml files. Parent directory must exist prior to calling this function, or
    * function will fail. 
    * 
    * @param frame Reference to the frame being saved
    * @param path The path to the folder where everything will be stored
    *  
    * @retval true  Successfully saved the data
    * @retval false Saving failed - for example, cannot create directory
    */
    static bool save(const RGBDFrame& frame, const std::string& path);

    /** @brief Loads the RGBD frame from disk. 
    * 
    * Loands the RGB and depth images from png, and the header and intrinsic matrix
    * from .yml files.  
    * 
    * @param frame Reference to the frame being loaded
    * @param path The path to the folder where everything was saved.
    *  
    * @retval true  Successfully loaded the data
    * @retval false Loading failed - for example, directory not found
    */
    static bool load(RGBDFrame& frame, const std::string& path);
    
  protected:

    /** @brief Constant for calculating std_dev(z) 
    * 
    * Khoshelham, K.; Elberink, S.O. Accuracy and Resolution of Kinect 
    * Depth Data for Indoor Mapping Applications. Sensors 2012, 12, 1437-1454.
    */
    static const double Z_STDEV_CONSTANT = 0.001425;
    
    /** @brief Calculates the var(z) from z
     * @param z the z depth, in meters
     * @return the variance in z, in meters^2
     */
    double getVarZ(double z);
    
    /** @brief Calculates the std_dev(z) from z
     * @param z the z depth, in meters
     * @return the standard deviation in z, in meters
     */
    double getStdDevZ(double z);

    /** @brief Calculates the z distribution (mean and variance) for a given pixel
     * 
     * Calculation is based on the standard quadratic model. See:
     *
     * Khoshelham, K.; Elberink, S.O. Accuracy and Resolution of Kinect 
     * Depth Data for Indoor Mapping Applications. Sensors 2012, 12, 1437-1454.
     * 
     * @param u the pixel u-coordinate
     * @param v the pixel v-coordinate
     * @param z_mean the mean of the distribution (will be the same az the z of the pixel), in meters
     * @param z_var var(z), will a quadratic function of the mean, in meters^2
     */    
    void getGaussianDistribution(int u, int v, double& z_mean, double& z_var);
    
    /** @brief Calculates the GMM z distribution (mean and variance) for a given pixel
     * 
     * Calculation is based on a Gaussian Mixture Model on top of
     * the standard quadratic model. The neigboring pixels contribute different
     * wights to the mixture. See:
     * 
     * Dryanovski et al ICRA2013 papaer
     * 
     * @todo reference for the paper
     * 
     * @param u the pixel u-coordinate
     * @param v the pixel v-coordinate
     * @param z_mean the mean of the distribution (will be the same az the z of the pixel), in meters
     * @param z_var var(z), will a quadratic function of the mean, in meters^2
     */   
    void getGaussianMixtureDistribution(int u, int v, double& z_mean, double& z_var);
};

} // namespace ccny_rgbd

#endif // CCNY_RGBD_RGBD_FRAME_H
