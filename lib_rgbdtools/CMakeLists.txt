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

#set(rgbdtools_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/rgbdtools")
set(rgbdtools_INSTALL_DIR "${CATKIN_DEVEL_PREFIX}") #/lib/rgbdtools")
set(rgbdtools_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${rgbdtools_INSTALL_DIR})
# message("rgbdtools_PREFIX='${rgbdtools_PREFIX}'")
# message("rgbdtools_INSTALL_DIR='${rgbdtools_INSTALL_DIR}'")
# message("rgbdtools_CMAKE_ARGS='${rgbdtools_CMAKE_ARGS}'")

set(rgbdtools_REPO_URI https://github.com/kpykc/rgbdtools.git)
set(rgbdtools_REPO_TAG master)
find_package(Git)
if(GIT_FOUND)
  message(WARNING "Git found: " ${GIT_EXECUTABLE})
  message(WARNING "Using *rgbdtools* repository with URI: " ${rgbdtools_REPO_URI} " TAG: " ${rgbdtools_REPO_TAG})

  include(ExternalProject)
  ExternalProject_Add(rgbdtools
    GIT_REPOSITORY ${rgbdtools_REPO_URI}
    GIT_TAG ${rgbdtools_REPO_TAG}
    #c4248d4094ec013fb4130f9a7740420a37bf601d
    #PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/
    PREFIX ${CMAKE_BINARY_DIR}/librgbdtools
    # CONFIGURE_COMMAND echo "No configure"
    # BUILD_COMMAND cmake
    # INSTALL_COMMAND make install INSTALL_PREFIX=${CATKIN_DEVEL_PREFIX}/lib/rgbdtools
    # BUILD_IN_SOURCE 1
    INSTALL_DIR ${rgbdtools_INSTALL_DIR}
    CMAKE_ARGS ${rgbdtools_CMAKE_ARGS}
  )
else()
  message(FATAL_ERROR "Git is not found, please try to set git executable path!")
endif()



