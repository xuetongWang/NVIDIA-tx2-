
file(GLOB utilSource *.cpp *.cu cammer/*.cpp cuda/*.cu display/*.cpp)
file(GLOB utilInclude *.h  cammer/*.h cuda/*.h display/*.h)



include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include)

cuda_add_library(util SHARED ${utilSource})
target_link_libraries(util stdc++fs GLEW gstreamer-1.0 gstapp-1.0)
