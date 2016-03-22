/*
 * GpuCache.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "GpuCache.h"

GpuCache::GpuCache() {
    this->device_id    = 0;
    this->page_size    = 0;
    this->slab_num     = 0;
    this->page_num     = 0;
    this->slabs        = NULL;
    this->grouped_slabs= NULL;
    this->initial_flag = -1;
    this->read_thread_num = 0;
}

GpuCache::GpuCache(const int64_t device_id) {
    this->device_id    = device_id;
    this->page_size    = 0;
    this->slab_num     = 0;
    this->page_num     = 0;
    this->slabs        = NULL;
    this->grouped_slabs= NULL;
    this->initial_flag = -1;
    this->read_thread_num = 0;
}

GpuCache::~GpuCache() {
    // TODO Auto-generated destructor stub
}

int64_t GpuCache::initial(const int64_t page_size, const int64_t page_num) {

    this->page_size = page_size;
    this->page_num  = page_num;
    this->slab_num  = (page_num * page_size)/1024;
    this->slabs     = new char*[slab_num];
    this->grouped_slabs= new char*[slab_num/1024];
    this->free_slab_num = 0;
   // this->cached_item.reserve(this->slab_num * 1.1);
    this->initial_flag = 0;
    return 0;
}

int64_t GpuCache::allocate_memory() {
//    CHECK_EQ(initial_flag, 0);
//    for (int64_t page_id = 0; page_id < page_num; ++page_id) {
//        mog_malloc_gpu(device_id, page_size, pages, page_id);
//    }
    CHECK_EQ(initial_flag, 0);
    free_slab_num = slab_num;
    for (int64_t grouped_slab_id = 0; grouped_slab_id < slab_num/1024; ++grouped_slab_id) {
        mog_malloc_gpu(device_id, 1024*1024, grouped_slabs, grouped_slab_id);
    }

    for (int64_t slab_id = 0; slab_id < slab_num; ++slab_id) {
        slabs[slab_id] = grouped_slabs[slab_id/1024] + (slab_id%1024)*1024;
        this->free_slabs.push(slab_id);
    }
    return 0;
}

int64_t GpuCache::insert(const std::string &key,
                         const int64_t length,
                         const char *value) {
    gpu_cache_writeLock lock_gpu_cache_insert(rw_cache_lock);
    
    bool push_flag = true;
    int64_t length_type = std::ceil(double(log2(length/1024.0)));
    if (length_type <= 0) {
        length_type = 0;
    }

    if (length_type > 10) {
        return -1;
    }

    if (this->cached_item.find(key) != this->cached_item.end()) {
        push_flag = false;
        this->free_slab_num += std::pow(2, this->cached_item[key].length_type);
        for (std::vector<int64_t>::iterator freed_slab = this->cached_item[key].slabs.begin();
            freed_slab != this->cached_item[key].slabs.end();
            ++freed_slab) {
            // memset(this->slabs[freed_slab], 0, 1024);
            this->free_slabs.push(*freed_slab);
        }    
    }

    if (this->free_slab_num < std::pow(2, length_type)) {
        while (true) {
            std::string deleted_item = this->cached_item_queue.front();
            this->cached_item_queue.pop();
            if (deleted_item == key) {
                push_flag = true;
                continue;
            }
            // this->free_slab_num.fetch_add(pow(2, this->cached_item[deleted_item].length_type));
            this->free_slab_num += std::pow(2, this->cached_item[deleted_item].length_type);
            for (std::vector<int64_t>::iterator freed_slab = this->cached_item[deleted_item].slabs.begin();
            		freed_slab != this->cached_item[deleted_item].slabs.end();
            		++freed_slab) {
                // memset(this->slabs[freed_slab], 0, 1024);
                this->free_slabs.push(*freed_slab);
            }
            this->cached_item.erase(deleted_item);
            if (this->free_slab_num >= std::pow(2, length_type)) {
                break;
            }
        }
    }

    this->cached_item[key].length = length;
    this->cached_item[key].length_type = length_type;

    int64_t new_slab_id = 0;
    for (int64_t i = 0; i < (std::pow(2, length_type)); ++i) {
        new_slab_id = this->free_slabs.front();
        this->cached_item[key].slabs.push_back(new_slab_id);
//        memcpy(this->slabs[new_slab_id], value + i*1024, 1024);
        mog_memcpy_cpu_to_gpu(this->device_id, this->slabs[new_slab_id], value + i*1024, 1024);
        this->free_slabs.pop();
        this->free_slab_num -= 1;
    }
    if (push_flag) {
        this->cached_item_queue.push(key);
    }
//    LOG(INFO) << "GPU CACHE INSERT " << key;
//    LOG(INFO) << "GPU CACHE FREE SLABS " << this->free_slab_num;
    return 0;
}

int64_t GpuCache::close() {
    this->cached_item.clear();
    for (int64_t grouped_slab_id = 0; grouped_slab_id < slab_num/1024; ++grouped_slab_id) {
        mog_free_gpu(device_id, grouped_slabs[grouped_slab_id]);
    }
    return 0;
}

int64_t GpuCache::read(const std::string& key, char *value) {

	std::map<std::string, GPUCachedItem>::iterator target_item = this->cached_item.find(key);
    //there is no such item in the cache
    if(target_item != this->cached_item.end()) {

        //find the target item in the cache
        LOG(INFO) << "GPU CACHE HIT " << key;
        gpu_cache_readLock lock_gpu_read(this->rw_cache_lock);

        std::vector<int64_t>::iterator i;

        for (i = target_item->second.slabs.begin(); i != target_item->second.slabs.end(); ++i) {
            mog_memcpy_gpu_to_gpu(this->device_id, value, this->slabs[*i], 1024);
            value += 1024;
        }
        return target_item->second.length;
    } else {
    	return 0;
    }

}

