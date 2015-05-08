cmake_minimum_required(VERSION 2.8)
project("autofocus")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
     message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set (LIBS)

include_directories("${CMAKE_SOURCE_DIR}/src/")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#
# You will need OpenCV with gPhoto2 wrapper
# can be downloaded from here https://github.com/dobrypd/opencv/tree/libgphoto2-connection
#

#find_package( OpenCV REQUIRED )
set(OpenCV_DIR "###############DIRECTORY WITH YOUR OPENCV INSTAL:ATION#############")
set(OpenCV_INCLUDE_DIRS "${OpenCV_DIR}/include")
set(OpenCV_LIBS_DIR "${OpenCV_DIR}/lib")
set(OpenCV_LIBRARIES opencv_core opencv_videoio opencv_highgui opencv_imgproc) 
link_directories(${OpenCV_LIBS_DIR})

include_directories(${OpenCV_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${OpenCV_LIBRARIES})

set (FILES_HXX)
set (FILES_CPP)

add_subdirectory(src)

add_executable(${PROJECT_NAME} ${FILES_HXX} ${FILES_CPP})

target_link_libraries(${PROJECT_NAME} ${LIBS})

