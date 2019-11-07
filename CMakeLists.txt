cmake_minimum_required(VERSION 2.8.3)
project(rosbag_record_pkg)

## C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  tf
  geometry_msgs
  cti_msgs
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rosbag.cpp    
    )
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rosbag.h   
    )
# add_library(THYAS ${SOURCES} ${HEADERS})

catkin_package()

include_directories(${catkin_INCLUDE_DIRS})
include_directories(include include)

add_executable(rosbag_record_pkg_node src/main.cpp ${HEADERS} ${SOURCES})
target_link_libraries(rosbag_record_pkg_node ${catkin_LIBRARIES})    
