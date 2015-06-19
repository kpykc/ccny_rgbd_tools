cmake_minimum_required(VERSION 2.8.3)
project(librgbdtools)


## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)

set(CATKIN_PACKAGE_INCLUDE_DESTINATION include/${PROJECT_NAME})
set(CATKIN_PACKAGE_LIB_DESTINATION lib)


###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
# catkin_package(
#   #INCLUDE_DIRS libLMS1xx
#   #LIBRARIES lms1xx
#   #CATKIN_DEPENDS roscpp sensor_msgs
#   DEPENDS system_lib
# )

#set(sfml_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/sfml")
set(rgbdtools_INSTALL_DIR "${CATKIN_DEVEL_PREFIX}") #/lib/rgbdtools")
set(rgbdtools_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${rgbdtools_INSTALL_DIR})
# message("sfml_PREFIX='${sfml_PREFIX}'")
# message("sfml_INSTALL_DIR='${sfml_INSTALL_DIR}'")
# message("sfml_CMAKE_ARGS='${sfml_CMAKE_ARGS}'")
find_package(Git)
if(GIT_FOUND)
  message("git found: ${GIT_EXECUTABLE}")
  include(ExternalProject)
  ExternalProject_Add(rgbdtools
    GIT_REPOSITORY https://github.com/kpykc/rgbdtools.git
    GIT_TAG c4248d4094ec013fb4130f9a7740420a37bf601d
    #PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/
    PREFIX ${CMAKE_BINARY_DIR}/librgbdtools
    # CONFIGURE_COMMAND echo "No configure"
    # BUILD_COMMAND cmake
    # INSTALL_COMMAND make install INSTALL_PREFIX=${CATKIN_DEVEL_PREFIX}/lib/ardrone
    # BUILD_IN_SOURCE 1
    INSTALL_DIR ${rgbdtools_INSTALL_DIR}
    CMAKE_ARGS ${rgbdtools_CMAKE_ARGS}
  )
else()
  message("Git is not found, please try to set git executable path!")
endif()



