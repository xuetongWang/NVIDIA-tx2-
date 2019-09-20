
file(GLOB maskcnnSource *.cpp)
file(GLOB maskcnnInclude *.h)

cuda_add_executable(mask-cnn ${maskcnnSource})

target_link_libraries(mask-cnn video)

