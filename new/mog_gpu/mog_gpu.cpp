//============================================================================
// Name        : DB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "mog_gpu.h"

extern char* mog_malloc_gpu(int64_t device_id, int64_t size);

//! DB constructor
mog_gpu::mog_gpu() {
	cuInit(0);
    CHECK_EQ(cuInit(0), CUDA_SUCCESS);
    get_device_num(&(this->device_num));
    CHECK_NE(this->device_num, 0);
    this->page_num = 100;
    this->page_size = 1024*1024;
}

mog_gpu::mog_gpu(const int64_t cache_size) {
	cuInit(0);
    CHECK_EQ(cuInit(0), CUDA_SUCCESS);
    get_device_num(&(this->device_num));
    CHECK_NE(this->device_num, 0);
    this->page_num = cache_size;
    this->page_size = 1024*1024;
}

//! DB constructor
/*!
  \param config_file configuration file path and name
*/

mog_gpu::~mog_gpu() {

}

int64_t mog_gpu::get_device_num(int *num) {
    CHECK_EQ(cuDeviceGetCount(num), CUDA_SUCCESS);
    CHECK_NE(device_num, 0);
    LOG(INFO) << "Find " << *num << " GPUs";
    return 0;
}

//! Initial parameter in the DB
/*!
  \param config_file configuration file path and name
  \return 0 if success
*/
int64_t mog_gpu::set_size(const int64_t cache_size) {
	//The default parameters
	this->page_num = cache_size;
	return 0;
}

int64_t mog_gpu::use_device(const int64_t device_id) {
    this->devices.push_back(device_id);
	return 0;
}

int64_t mog_gpu::setup() {
	std::vector<int64_t>::iterator device_id;
    for (device_id = this->devices.begin(); device_id != this->devices.end(); ++device_id) {
        this->gpu_caches[*device_id] = new GpuCache(*device_id);
        CHECK_EQ(gpu_caches[*device_id]->initial(this->page_size, this->page_num), 0);
        CHECK_EQ(gpu_caches[*device_id]->allocate_memory(), 0);
        LOG(INFO) << "---------> Create GPU Cache Done On Device " << *device_id;
    }
    return 0;
}

int64_t mog_gpu::close() {
	std::vector<int64_t>::iterator device_id;
	for (device_id = this->devices.begin(); device_id != this->devices.end(); ++device_id) {
        this->gpu_caches[*device_id]->close();
    }
    LOG(INFO) << "Clean GPU Cache Data\n";
    return 0;
}

char* mog_gpu::malloc(int64_t device_id, int64_t size) {
    return mog_malloc_gpu(device_id, size*1024*1024);
}

int64_t mog_gpu::read(const int64_t device_id, const std::string& key, char *value) {
    int64_t length = 0;
    length = this->gpu_caches[device_id]->read(key, value);
    return length;
}

int64_t mog_gpu::insert(const int64_t device_id, const std::string& key, const std::string& value) {
	this->gpu_caches[device_id]->insert(key, value.length(), value.c_str());
	return 0;
}

