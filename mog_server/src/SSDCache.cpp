/*
 * SSDCache.cpp
 *
 *  Created on: 24 Jan 2016
 *      Author: sunp
 */

#include "SSDCache.h"

namespace cap {

SSDCache::SSDCache() {
  max_block_num = 0;
  block_num = 0;
  thread_pool = new boost::threadpool::pool(12);
  read_num = 0;
}

SSDCache::~SSDCache() {
}

int64_t SSDCache::initial() {
  block_num = 0;
  thread_pool = new boost::threadpool::pool(12);
  read_num = 0;
  cached_block.reserve(max_block_num*1.5);
  unsynced_block_id.reserve(max_block_num*1.5);
  return 0;
}

void SSDCache::flush(int64_t block_id) {
  std::string src_name = SSDCache::singleton().ssd_path + "/" + std::to_string(block_id);
  std::string dst_name = SSDCache::singleton().hdd_path + "/" + std::to_string(block_id);
  boost::filesystem::copy_file(boost::filesystem::path(src_name), boost::filesystem::path(dst_name));

  ssd_cache_writeLock flush_lock(SSDCache::singleton().ssd_cache_rwlock);
  int64_t target_file_size = SSDCache::singleton().unsynced_block_id[block_id];
  SSDCache::singleton().unsynced_block_id.erase(block_id);
  SSDCache::singleton().cached_block_id.push_back(block_id);
  SSDCache::singleton().cached_block[block_id] = target_file_size;
//LOG(INFO) << "FLUSH BLOCK " << block_id << " FROM SSD TO hdd";

  if (SSDCache::singleton().block_num >= SSDCache::singleton().max_block_num) {
    while (1) {
      if (SSDCache::singleton().cached_block_id.size() == 0) {
        break;
      }
      if (SSDCache::singleton().block_num < 0.7 * SSDCache::singleton().max_block_num) {
        break;
      }
      int64_t deleted_i = SSDCache::singleton().cached_block_id.front();
//    LOG(INFO) << "SSD CACHE DELETE " << deleted_i;
      SSDCache::singleton().cached_block.erase(deleted_i);
      SSDCache::singleton().cached_block_id.pop_front();
      std::string deleted_name = SSDCache::singleton().ssd_path + "/" + std::to_string(deleted_i);
      boost::filesystem::path deleted_path(deleted_name);
      boost::filesystem::remove(deleted_path);
      SSDCache::singleton().block_num.fetch_sub(1);
//    LOG(INFO) << "SSD CACHE BLOCK NUM: " << SSDCache::singleton().block_num;
    }
  }
}

void SSDCache::fetch(int64_t block_id) {

  if (SSDCache::singleton().block_num < SSDCache::singleton().max_block_num) {
    std::string src_name = SSDCache::singleton().hdd_path + "/" + std::to_string(block_id);
    std::string dst_name = SSDCache::singleton().ssd_path + "/" + std::to_string(block_id);
    boost::filesystem::copy_file(boost::filesystem::path(src_name), boost::filesystem::path(dst_name));
    ssd_cache_writeLock fetch_lock(SSDCache::singleton().ssd_cache_rwlock);
    boost::filesystem::path dst_path(dst_name);
    int64_t target_file_size = boost::filesystem::file_size(dst_path);
    SSDCache::singleton().cached_block_id.push_back(block_id);
    SSDCache::singleton().cached_block[block_id] = target_file_size;
//  LOG(INFO) << "FETCH BLOCK " << block_id << " FROM hdd TO SSD";

    SSDCache::singleton().block_num.fetch_add(1);
    if (SSDCache::singleton().block_num >= SSDCache::singleton().max_block_num) {
//    LOG(INFO) << "Clean SSD Cache: BLOCKS->" << SSDCache::singleton().block_num
      while (1) {
        if (SSDCache::singleton().cached_block_id.size() == 0) {
          break;
        }
        if (SSDCache::singleton().block_num < 0.7 * SSDCache::singleton().max_block_num) {
          break;
        }
        int64_t deleted_i = SSDCache::singleton().cached_block_id.front();
        SSDCache::singleton().cached_block.erase(deleted_i);
        SSDCache::singleton().cached_block_id.pop_front();
        std::string deleted_name = SSDCache::singleton().ssd_path + "/" + std::to_string(deleted_i);
        boost::filesystem::path deleted_path(deleted_name);
        boost::filesystem::remove(deleted_path);
        SSDCache::singleton().block_num.fetch_sub(1);
//      LOG(INFO) << "SSD CACHE DELETE " << deleted_i;
//      LOG(INFO) << "SSD CACHE BLOCK NUM: " << SSDCache::singleton().block_num;
      }
    }
    SSDCache::singleton().caching_block.erase(block_id);
  }
}

SSDCache& SSDCache::singleton() {
  static SSDCache ssd_cache;
  return ssd_cache;
}

int64_t SSDCache::new_block(int64_t block_id) {
  ssd_cache_writeLock new_block_lock(SSDCache::singleton().ssd_cache_rwlock);
  //this->unsynced_block_lock.lock();
  boost::filesystem::path target_path(this->ssd_path + "/" + std::to_string(block_id));
  this->unsynced_block_id[block_id] = boost::filesystem::file_size(target_path);
  this->block_num.fetch_add(1);

  auto temp_func = std::bind(SSDCache::flush, block_id);
  this->thread_pool->schedule(temp_func);
//this->unsynced_block_lock.unlock();
  return 0;
}

int64_t SSDCache::sync() {
  while(this->thread_pool->pending() > 0 || this->thread_pool->active() > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  while (SSDCache::singleton().unsynced_block_id.size() > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  return 0;
}

int64_t SSDCache::close() {
  this->sync();
  this->unsynced_block_id.clear();
  this->cached_block.clear();
  this->cached_block_handle_id.clear();
  this->caching_block.clear();
  this->cached_block_id.clear();
  this->read_record.clear();
  delete thread_pool;
  return 0;
}

int64_t SSDCache::read(const std::string &key, const IndexInfo &key_info, char* value) {
  ssd_cache_readLock read_lock(SSDCache::singleton().ssd_cache_rwlock);
  this->read_record_lock.lock();
  if (this->read_record.find(key_info.block_id) == this->read_record.end()) {
    this->read_record[key_info.block_id] = 0;
  }
  ++this->read_record[key_info.block_id];
  this->read_record_lock.unlock();
  this->read_num.fetch_add(1);

  int64_t check_threshold = 50;
  if (this->read_num > check_threshold) {
    //fetching blocks from hdd and store it in ssd
    this->read_record_lock.lock();
    std::map<std::int64_t, std::vector<std::int64_t>> read_record_sta;
    for (auto i : this->read_record) {
      read_record_sta[check_threshold - i.second].push_back(i.first);
    }
    int64_t block_id_to_fetch = 0;
    for (auto block_bucket : read_record_sta) {
      for(auto candidate_block : block_bucket.second) {
        if (this->cached_block.find(candidate_block) == this->cached_block.end())
          block_id_to_fetch = candidate_block;
      }
    }
    if (block_id_to_fetch != 0) {
      //fetch a block id
      if (this->cached_block.find(block_id_to_fetch) == this->cached_block.end() &&
          this->caching_block.find(block_id_to_fetch) == this->caching_block.end()) {
//      LOG(INFO) << "SSD is going to fetch Block: " << block_id_to_fetch;
        this->caching_block[block_id_to_fetch] = 1;
        auto temp_func = std::bind(SSDCache::fetch, block_id_to_fetch);
        this->thread_pool->schedule(temp_func);
      }
    }
    this->read_num = 0;
    this->read_record.clear();
    this->read_record_lock.unlock();
  }
  if (this->cached_block.find(key_info.block_id) != this->cached_block.end() ||
      this->unsynced_block_id.find(key_info.block_id) != this->unsynced_block_id.end()) {
//  LOG(INFO) << "SSD CACHE HIT: " << key;
    std::string target_file = this->ssd_path + "/" + std::to_string(key_info.block_id);
    FILE *p_file = fopen(target_file.c_str(), "rb");
    if (p_file == NULL) {
//    LOG(FATAL) << "CANNOT FIND FILE IN SSD->" << target_file;
    }
    fseek(p_file, key_info.offset, 0);
    fread((void*)value, 1, key_info.length, p_file);
    fclose(p_file);
    return key_info.length;
  } else {
    //LOG(INFO) << "HDD STORAGE HIT: " << key;
    std::string target_file = this->hdd_path + "/" + std::to_string(key_info.block_id);
    FILE *p_file = fopen(target_file.c_str(), "rb");
    if (p_file == NULL) {
      LOG(FATAL) << "CANNOT FIND FILE IN HDD";
    }
    fseek(p_file, key_info.offset, 0);
    fread((void*)value, 1, key_info.length, p_file);
    fclose(p_file);
    return key_info.length;
  }
}

} /* namespace cap */
