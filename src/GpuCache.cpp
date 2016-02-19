/*
 * GpuCache.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "GpuCache.h"

namespace cap {

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

GpuCache::GpuCache(const int32_t device_id) {
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

int32_t GpuCache::initial(const int32_t page_size, const int32_t page_num) {

    this->page_size = page_size;
    this->page_num  = page_num;
    this->slab_num  = (page_num * page_size)/1024;
    this->slabs     = new char*[slab_num];
    this->grouped_slabs= new char*[slab_num/1024];
    this->free_slab_num = 0;

    this->cached_item.reserve(this->slab_num * 1.1);

    this->initial_flag = 0;
    return GPUCACHE_SUCCESS;
}

int32_t GpuCache::allocate_memory() {
//    CHECK_EQ(initial_flag, 0);
//    for (int32_t page_id = 0; page_id < page_num; ++page_id) {
//        mog_malloc_gpu(device_id, page_size, pages, page_id);
//    }
    CHECK_EQ(initial_flag, 0);
    free_slab_num = slab_num;

    for (int32_t grouped_slab_id = 0; grouped_slab_id < slab_num/1024; ++grouped_slab_id) {
        mog_malloc_gpu(device_id, 1024*1024, grouped_slabs, grouped_slab_id);
    }

    for (int32_t slab_id = 0; slab_id < slab_num; ++slab_id) {
        slabs[slab_id] = grouped_slabs[slab_id/1024] + (slab_id%1024)*1024;
        this->free_slabs.push(slab_id);
    }


    return GPUCACHE_SUCCESS;
}

int32_t GpuCache::insert(const std::string &key,
        const int32_t length,
        const int32_t length_type,
        char *value) {

    gpu_cache_writeLock lock_gpu_cache_insert(rw_cache_lock);

    LOG(INFO) << "GPU CACHE TRY INSERT " << key;

    if (this->free_slab_num < pow(2, length_type)) {
        while (true) {
            std::string deleted_item = this->cached_item_queue.front();
            this->cached_item_queue.pop();

            this->free_slab_num.fetch_add(pow(2, this->cached_item[deleted_item].length_type));

            for (auto freed_slab : this->cached_item[deleted_item].slabs) {
               // memset(this->slabs[freed_slab], 0, 1024);
                this->free_slabs.push(freed_slab);
            }

            this->cached_item.erase(deleted_item);
            LOG(INFO) << "GPU CACHE DELETE: " << deleted_item;
            LOG(INFO) << "GPU CACHE FREE SLABS " << this->free_slab_num;

            if (this->free_slab_num >= pow(2, length_type)) {
                break;
            }
        }
    }

    this->cached_item[key].length = length;
    this->cached_item[key].length_type = length_type;

    int32_t new_slab_id = 0;
    for (int32_t i = 0; i < (pow(2, length_type)); ++i) {
        new_slab_id = this->free_slabs.front();
        this->cached_item[key].slabs.push_back(new_slab_id);
//        memcpy(this->slabs[new_slab_id], value + i*1024, 1024);
        mog_memcpy_cpu_to_gpu(this->device_id, this->slabs[new_slab_id], value + i*1024, 1024);
        this->free_slabs.pop();
        this->free_slab_num.fetch_sub(1);
    }

    this->cached_item_queue.push(key);

    LOG(INFO) << "GPU CACHE INSERT " << key;
    LOG(INFO) << "GPU CACHE FREE SLABS " << this->free_slab_num;

    return 0;
}

int32_t GpuCache::close() {
    this->cached_item.clear();

    for (int32_t grouped_slab_id = 0; grouped_slab_id < slab_num/1024; ++grouped_slab_id) {
        mog_free_gpu(device_id, grouped_slabs[grouped_slab_id]);
    }

    return 0;
}

int32_t GpuCache::read(const std::string& key, char *value) {

    auto target_item = this->cached_item.find(key);

    //there is no such item in the cache
    if(target_item == this->cached_item.end()) {

        LOG(INFO) << "GPU CACHE MISS " << key;

        this->read_thread_num.fetch_add(1);

        int32_t length = 0;
        int32_t buffer_id = 0;

        length = WriteBuffer::singleton().read(key, this->cpu_buffer[buffer_id]);

        if (length == 0) {
            length = CpuCache::singleton().read(key, this->cpu_buffer[buffer_id]);
        }

        int32_t length_type = std::ceil(std::log2(length/1024.0));
        if (length_type < 0) {
            length_type = 0;
        }

        if (length != 0) {
            this->insert(key, length, length_type, this->cpu_buffer[buffer_id]);

            gpu_cache_readLock lock_gpu_read(this->rw_cache_lock);

            target_item = this->cached_item.find(key);
            for (auto i : target_item->second.slabs) {
                mog_memcpy_gpu_to_gpu(this->device_id, value, this->slabs[i], 1024);
                value += 1024;
            }
        }
        this->read_thread_num.fetch_sub(1);
        return length;

    } else {
        //find the target item in the cache

        LOG(INFO) << "GPU CACHE HIT " << key;

        gpu_cache_readLock lock_gpu_read(this->rw_cache_lock);

        for (auto i : target_item->second.slabs) {
            mog_memcpy_gpu_to_gpu(this->device_id, value, this->slabs[i], 1024);
            value += 1024;
        }
        return target_item->second.length;
    }
}



} /* namespace cap */
