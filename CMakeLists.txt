#=============================================================================
# Copyright 2013 de Weber, Henrique.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#
# Last updated 24/10/2013
#=============================================================================

project(ShadowDetector)

cmake_minimum_required(VERSION 2.8)
#set (CMAKE_CXX_FLAGS "-std=c++0x -stdlib=libc++ -g3 -Wall -O0")
set (CMAKE_CXX_FLAGS "-std=c++0x -g3 -Wall -O0")
set(CMAKE_PREFIX_PATH /opt/local/lib/cmake/)
include_directories(/home/gopro/Sandbox/ShadowDetector/src)
include_directories(/home/gopro/Desktop/cvblob/cvBlob)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

SET(CMAKE_C_FLAGS -std=gnu99)
#SET(CMAKE_CXX_FLAGS -std=gnu++11)

# tell cmake that the binaries goes in the binary directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)

# required packages
FIND_PACKAGE(OpenCV REQUIRED)

add_executable(ShadowDetector ${CMAKE_SOURCE_DIR}/src/MainShadowDetector.cpp
						            ${CMAKE_SOURCE_DIR}/src/SquareDetection.cpp
            ${CMAKE_SOURCE_DIR}/src/RobustMatcher.cpp
            /home/gopro/Desktop/cvblob/cvBlob/cvlabel.cpp
            /home/gopro/Desktop/cvblob/cvBlob/cvblob.cpp
						${CMAKE_SOURCE_DIR}/src/ImageManipulation.cpp
            ${CMAKE_SOURCE_DIR}/src/ShadowDetector.cpp
            ${CMAKE_SOURCE_DIR}/src/MathFunctions.cpp
			${CMAKE_SOURCE_DIR}/src/PlateExtraction.cpp
                        )

TARGET_LINK_LIBRARIES(ShadowDetector ${OpenCV_LIBS})


