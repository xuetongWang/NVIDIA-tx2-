cmake_minimum_required(VERSION 2.8)

project(video)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

set(CMAKE_BUILD_TYPE "Debug")

message("sleton use cc :${CMAKE_CXX_FLAGS}")

#CUDA
find_package(CUDA)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
set(
        CUDA_NVCC_FLAGS
        ${CUDA_NVCC_FLAGS};
    -O3
        -gencode arch=compute_62,code=sm_62
)

include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include
                    /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/
                    util/)

file(GLOB videoSource *.cpp *.cu)
file(GLOB videoInclude *.h)


cuda_add_library(video SHARED ${videoSource})

add_subdirectory(mask_cnn)
add_subdirectory(yolo)
add_subdirectory(util)

target_link_libraries(video util Qt4::QtGui)
