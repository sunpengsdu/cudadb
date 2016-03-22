cuda_add_library(MogGPU mog_GPU.cu)

#aux_source_directory(. DIR_SRCS)
cuda_add_executable(Test_MOG main.cpp CentraIndex.cpp GpuCache.cpp SSDCache.cpp CpuCache.cpp WriteBuffer.cpp mog.cpp)

target_link_libraries(Test_MOG ${MogGPU}  ${CUDA_LIBRARIES} ${libglog} ${libleveldb} ${libyaml-cpp} ${libcuda} ${Boost_LIBRARIES})
