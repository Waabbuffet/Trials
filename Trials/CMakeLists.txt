cmake_minimum_required(VERSION 3.10)

#Project Name
project(Capstone VERSION 1.0)

#find directories and setup requirements
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#This will include the trials package
include_directories(${PROJECT_SOURCE_DIR})

#Start the executable
add_executable(Capstone Main.cpp)

#Include openCL
find_package(OpenCL REQUIRED)
find_library(OpenCL_LIBRARY OpenCL)
if(OpenCL_FOUND)
	include_directories(${OpenCL_INCLUDE_DIRS})
	target_link_libraries (Capstone ${OpenCL_LIBRARY})
endif(OpenCL_FOUND)

#Include openCV
find_package(OpenCV REQUIRED)
find_library(OpenCV_LIBRARY OpenCV)
if(OpenCV_FOUND)
	include_directories(${OpenCV_INCLUDE_DIRS})
	target_link_libraries (Capstone ${OpenCV_LIBRARY})
endif(OpenCV_FOUND)


#Include EVPP
find_path(EVPP_INCLUDE_DIR evpp)
find_library(EVPP_LIBRARY evpp_static)
include_directories(${EVPP_INCLUDE_DIR})
target_link_libraries (Capstone ${EVPP_LIBRARY})

#Include GLOG for EVPP
find_path(GLOG_INCLUDE_DIR glog)
find_library(GLOG_LIBRARY glog)
include_directories(${GLOG_INCLUDE_DIR})
target_link_libraries (Capstone ${GLOG_LIBRARY})

#Include LIBEVENT for EVPP
#Libevent consists of 3 parts
find_path(LIBEVENT_INCLUDE_DIR event2/event.h)
find_library(LIBEVENT_LIBRARY event)
find_library(LIBEVENTCORE_LIBRARY event_core)
find_library(LIBEVENTSTATIC_LIBRARY event_extra)
 
include_directories(${LIBEVENT_INCLUDE_DIR})
target_link_libraries (Capstone ${LIBEVENT_LIBRARY})
target_link_libraries (Capstone ${LIBEVENTCORE_LIBRARY})
target_link_libraries (Capstone ${LIBEVENTSTATIC_LIBRARY})

#Setup options and configure files
if(UNIX)
	option(H_OS_WINDOWS "Host OS is windows" OFF)
else()
	option(H_OS_WINDOWS "Host OS is windows" ON)
endif(UNIX)

configure_file(Main.h.in Main.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})



