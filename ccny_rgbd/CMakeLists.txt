cmake_minimum_required(VERSION 2.4.6)
project(ccny_rgbd)

find_package(catkin REQUIRED dynamic_reconfigure roscpp 
  # octomap
  image_transport
  sensor_msgs
  cv_bridge
  message_generation
  tf
  )
find_package(Boost REQUIRED COMPONENTS signals filesystem system regex )
find_package(OpenCV REQUIRED)

add_service_files(
  FILES
  AddManualKeyframe.srv
  GenerateGraph.srv
  Load.srv
  PublishKeyframe.srv
  PublishKeyframes.srv
  Save.srv
  SolveGraph.srv
)

generate_messages()

generate_dynamic_reconfigure_options(
  ./cfg/FeatureDetector.cfg
  ./cfg/GftDetector.cfg
  ./cfg/OrbDetector.cfg
  ./cfg/RGBDImageProc.cfg
  ./cfg/StarDetector.cfg
)

include_directories(
  include/
  ${Boost_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS
  roscpp
  #  octomap
  DEPENDS
)

################################################################
# Dependencies
################################################################

# OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
link_libraries(${OpenCV_LIBRARIES})

# Octomap
find_package(octomap REQUIRED)
include_directories(${OCTOMAP_INCLUDE_DIRS})
link_directories(${OCTOMAP_LIBRARY_DIRS})
link_libraries(${OCTOMAP_LIBRARIES})

# PCL
find_package(PCL REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
link_libraries(${PCL_LIBRARIES})

#g2o
set(G2O_LIBRARIES 
  g2o_core 
  g2o_stuff
  g2o_solver_cholmod 
  g2o_solver_csparse 
  g2o_types_slam3d
  cholmod)

################################################################
# Build visual odometry application
################################################################


add_executable(visual_odometry_node 
  src/node/visual_odometry_node.cpp
  src/apps/visual_odometry.cpp
  src/util.cpp)

add_dependencies(visual_odometry_node ${PROJECT_NAME}_gencfg)
  
target_link_libraries (visual_odometry_node
  rgbdtools
  boost_signals
  boost_system
  boost_filesystem
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES})
  
################################################################
# Build keyframe mapper application
################################################################

add_executable(keyframe_mapper_node
  src/node/keyframe_mapper_node.cpp
  src/apps/keyframe_mapper.cpp
  src/util.cpp)

add_dependencies(keyframe_mapper_node ${PROJECT_NAME}_gencfg)
  
target_link_libraries (keyframe_mapper_node
  rgbdtools
  boost_signals
  boost_system
  boost_filesystem
  boost_regex
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${G2O_LIBRARIES})
  
################################################################
# Build feature viewer application
################################################################

add_executable(feature_viewer_node 
  src/node/feature_viewer_node.cpp
  src/apps/feature_viewer.cpp
  src/util.cpp)

add_dependencies(feature_viewer_node ${PROJECT_NAME}_gencfg)
  
target_link_libraries (feature_viewer_node
  rgbdtools
  boost_signals
  boost_system
  boost_filesystem
  ${catkin_LIBRARIES}
  ${PCL_LIBRARIES}
  ${OpenCV_LIBRARIES})

  
################################################################
# Build image proc application
################################################################

add_library(rgbd_image_proc_app 
  src/apps/rgbd_image_proc.cpp
  src/util.cpp)

target_link_libraries (rgbd_image_proc_app  
  rgbdtools
  boost_signals
  boost_system
  boost_filesystem
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES})
  
add_executable(rgbd_image_proc_node src/node/rgbd_image_proc_node.cpp)
add_library(rgbd_image_proc_nodelet src/nodelet/rgbd_image_proc_nodelet.cpp)
  
target_link_libraries(rgbd_image_proc_node    rgbd_image_proc_app ${catkin_LIBRARIES})
target_link_libraries(rgbd_image_proc_nodelet rgbd_image_proc_app ${catkin_LIBRARIES})

install(TARGETS visual_odometry_node 
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(TARGETS keyframe_mapper_node
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(TARGETS feature_viewer_node 
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(TARGETS rgbd_image_proc_node 
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(TARGETS rgbd_image_proc_app 
        DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)
install(TARGETS rgbd_image_proc_nodelet
        DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)


