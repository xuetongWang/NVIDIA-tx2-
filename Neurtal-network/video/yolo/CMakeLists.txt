
file(GLOB yoloSource *.cpp network/*.cpp)
file(GLOB yoloInclude *.h network/*.h)

cuda_add_executable(yolo ${yoloSource})
target_link_libraries(yolo video nvinfer nvcaffe_parser nvinfer_plugin)

