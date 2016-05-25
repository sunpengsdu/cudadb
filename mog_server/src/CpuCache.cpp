/*
 * CpuCache.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "CpuCache.h"

namespace cap {

const int64_t CpuCache::slab_size[11] = {1*1024, 2*1024, 4*1024, 8*1024, 16*1024,
                                         32*1024, 64*1024, 128*1024, 256*1024,
                                         512*1024, 1024*1024};

CpuCache::CpuCache() {
  page_size = 0;
  page_num  = 0;
  slabs     = NULL;
  initial_flag = -1;
}

CpuCache::~CpuCache() {
  // TODO Auto-generated destructor stub
}

CpuCache& CpuCache::singleton() {
  static CpuCache cpu_cache;
  return cpu_cache;
}

int64_t CpuCache::initial(const int64_t page_size, const int64_t page_num) {
  this->page_size = page_size;
  this->page_num  = page_num;
  this->slab_num  = page_num * page_size / 1024;
  this->slabs     = new char*[this->slab_num];
  this->free_slab_num = 0;
  this->cached_item.reserve(this->slab_num * 1.1);
  this->initial_flag = 0;
  return CPUCACHE_SUCCESS;
}

int64_t CpuCache::allocate_memory() {
  CHECK_EQ(initial_flag, 0);
  free_slab_num = slab_num;
  for (int64_t slab_id = 0; slab_id < slab_num; ++slab_id) {
    slabs[slab_id] = new char[1024];
    memset(slabs[slab_id], 0, 1024);
    this->free_slabs.push(slab_id);
  }
  return CPUCACHE_SUCCESS;
}

int64_t CpuCache::insert(const std::string &key, const IndexInfo &central_index_info, char *value) {
  cpu_cache_writeLock lock_cpu_cache_insert(rw_cache_lock);
  if (this->free_slab_num < pow(2, central_index_info.length_type)) {
    return 0;
   //Try to replace cached data
    while (true) {
      std::string deleted_item = this->cached_item_queue.front();
      this->cached_item_queue.pop();
      this->free_slab_num.fetch_add(pow(2, this->cached_item[deleted_item].length_type));
      for (auto freed_slab : this->cached_item[deleted_item].slabs) {
        memset(this->slabs[freed_slab], 0, 1024);
        this->free_slabs.push(freed_slab);
      }
      this->cached_item.erase(deleted_item);
      if (this->free_slab_num >= pow(2, central_index_info.length_type)) {
        break;
      }
    }
  }
  this->cached_item[key].length = central_index_info.length;
  this->cached_item[key].length_type = central_index_info.length_type;

  int64_t new_slab_id = 0;
  for (int64_t i = 0; i < (pow(2, central_index_info.length_type)); ++i) {
    new_slab_id = this->free_slabs.front();
    this->cached_item[key].slabs.push_back(new_slab_id);
    memcpy(this->slabs[new_slab_id], value + i*1024, 1024);
    this->free_slabs.pop();
    this->free_slab_num.fetch_sub(1);
  }
  this->cached_item_queue.push(key);
//LOG(INFO) << "RAM CACHE INSERT " << key;
//LOG(INFO) << "RAM CACHE FREE SLABS " << this->free_slab_num;
  return 0;
}

int64_t CpuCache::close() {
  this->cached_item.clear();
  for (int64_t slab_id = 0; slab_id < slab_num; ++slab_id) {
    delete[] slabs[slab_id];
  }
  delete[] slabs;
  while(!this->cached_item_queue.empty()) {
    this->cached_item_queue.pop();
  }
  while(!this->free_slabs.empty()) {
    this->free_slabs.pop();
  }
  slabs     = NULL;
  initial_flag = -1;
  return 0;
}

int64_t CpuCache::read(const std::string& key, char *value) {
  auto target_item = this->cached_item.find(key);
  //there is no such item in the cache
  if(target_item == this->cached_item.end()) {
    IndexInfo key_centra_index;
    int64_t index_read_length = 0;
    //find in central index
    index_read_length = CentraIndex::singleton().get(key.c_str(), 
        key.length(), (char*)&key_centra_index, sizeof(IndexInfo));
    if (index_read_length != sizeof(IndexInfo)) {
      return 0;
    } else {
      SSDCache::singleton().read(key, key_centra_index, value);
      this->insert(key, key_centra_index, value);
      return key_centra_index.length;
    }
  } else {
    //find the target item in the cache
    // LOG(INFO) << "RAM CACHE HIT " << key;
    cpu_cache_readLock lock_ram_read(this->rw_cache_lock);
    for (auto i : target_item->second.slabs) {
      memcpy(value, this->slabs[i], 1024);
      value += 1024;
    }
    return target_item->second.length;
  }
  return 0;
}
} /* namespace cap */
