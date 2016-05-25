/*
 * writebuffers.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "WriteBuffer.h"

namespace cap {

const int64_t WriteBuffer::slab_size[11] = {1*1024, 2*1024, 4*1024, 8*1024, 16*1024,
                                            32*1024, 64*1024, 128*1024, 256*1024,
                                            512*1024, 1024*1024};

WriteBuffer::WriteBuffer() {
  this->page_size      = 0;
  this->page_per_block = 0;
  this->page_num       = 0;
  this->max_page_num   = 0;
  this->initial_flag   = -1;
  this->latest_block_id = 0;
  this->flush_thread_num = 10;
}

WriteBuffer::~WriteBuffer() {
}

WriteBuffer& WriteBuffer::singleton() {
  static WriteBuffer write_buffer;
  return write_buffer;
}
int64_t WriteBuffer::initial(const int64_t page_size, const int64_t page_per_block, const int64_t page_num) {
  this->total_size_without_block = 0;
  this->total_size        = 0;
  this->page_size         = page_size;
  this->latest_block_id   = 0;
  this->page_per_block    = page_per_block;
  this->page_num          = page_num;
  this->max_page_num      = int64_t(1.2 * page_num);
  this->initial_flag      = 0;
  this->flush_thread_num  = 10;
  this->flush_thread_pool = new boost::threadpool::pool(flush_thread_num);
  this->kv_store.reserve(this->max_page_num*1024*1.2);
  this->blocks.reserve(this->max_page_num*1.2);
  return WRITEBUFFER_SUCCESS;
}

int64_t WriteBuffer::allocate_memory() {
  CHECK_EQ(initial_flag, 0);
  return WRITEBUFFER_SUCCESS;
}

int64_t WriteBuffer::write(const std::string& key, const char *value, int64_t length) {
  if (length > WriteBuffer::slab_size[10]) {
    return -1;
  }
  while (this->total_size > this->max_page_num * this->page_size) {
    //std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  while (SSDCache::singleton().block_num >= SSDCache::singleton().max_block_num) {
    // std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  wbuffer_writeLock write_lock(this->wbuffer_lock);
  size_t hashed_key  = std::hash<std::string>()(key);
  auto existed_key = this->kv_store.find(hashed_key);
  bool existence = false;
  if (existed_key != this->kv_store.end()) {
    return -1;
  }
  this->kv_store[hashed_key].key  = key;
  this->kv_store[hashed_key].value_length = length;
  int64_t target_slab_type = std::ceil(std::log2(length/1024.0));
  //if the computed target_slab_type is less than 0
  //use the smallest slab type: 2^0 * 1024 (1KB)
  if (target_slab_type < 0) {
    target_slab_type = 0;
  }
  this->kv_store[hashed_key].value_length_type = target_slab_type;
  this->kv_store[hashed_key].value = new char[WriteBuffer::slab_size[target_slab_type]];
  memcpy(this->kv_store[hashed_key].value, value, length);

  if (existence == false) {
    this->kv_list.push(hashed_key);
  }
  this->total_size.fetch_add(WriteBuffer::slab_size[target_slab_type]);
  this->total_size_without_block.fetch_add(WriteBuffer::slab_size[target_slab_type]);
  if (this->total_size_without_block > (this->page_per_block*1024*1024)) { // 1 page 1 MB
    ++latest_block_id;
    int64_t new_block_size = 0;
    size_t temp_hashed_key = 0;
    int64_t temp_slab_type;
    bool w_lock = false;
    while (true) {
      temp_hashed_key = this->kv_list.front();
      temp_slab_type  = this->kv_store[temp_hashed_key].value_length_type;
      if (new_block_size + WriteBuffer::slab_size[temp_slab_type] >
          this->page_per_block*1024*1024) {
        break;
      } else {
        this->kv_list.pop();
        this->blocks[latest_block_id].push_back(this->kv_store[temp_hashed_key]);
        this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[temp_slab_type]);
        new_block_size += WriteBuffer::slab_size[temp_slab_type];
      }
    }
    auto flush_func = std::bind(WriteBuffer::flush, latest_block_id);
    flush_thread_pool->schedule(flush_func);
  }
  return WRITEBUFFER_SUCCESS;
}

void WriteBuffer::flush(int64_t block_id) {
  int64_t flushed_size = 0;
  int64_t key_num = 0;
  std::string string_block_id = std::to_string(block_id);
  //*********
  //hdd or ssd ?
  //std::string fout_name = WriteBuffer::singleton().ssd_path + "/" + string_block_id;
  std::string fout_name = WriteBuffer::singleton().hdd_path + "/" + string_block_id;
  boost::filesystem::path flushed_path = boost::filesystem::path(fout_name);
  if (boost::filesystem::exists(flushed_path) == true) {
    boost::filesystem::remove(flushed_path);
  }
  std::ofstream fout(fout_name);
  int64_t offset = 0;
  IndexInfo new_record;
  new_record.block_id = block_id;
  wbuffer_writeLock write_lock(WriteBuffer::singleton().wbuffer_lock);
  for (auto &i : WriteBuffer::singleton().blocks[block_id]) {
    flushed_size += WriteBuffer::slab_size[i.value_length_type];
    ++key_num;
    fout.write(i.value, i.value_length);
    new_record.offset = offset;
    new_record.length = i.value_length;
    new_record.length_type = i.value_length_type;
    CentraIndex::singleton().put(i.key.c_str(), i.key.length(), (char*)&new_record, sizeof(new_record));
    offset += i.value_length;
    delete[] i.value;
    WriteBuffer::singleton().kv_store.erase(std::hash<std::string>()(i.key));
  }
  fout.close();
  /*
      SSDCache::singleton().new_block(block_id);
  */
  WriteBuffer::singleton().blocks.erase(block_id);
  WriteBuffer::singleton().total_size.fetch_sub(flushed_size);
}

int64_t WriteBuffer::sync() {
  while (! this->kv_list.empty()) { // 1 page 1 MB
    ++latest_block_id;
    int64_t new_block_size = 0;
    size_t temp_hashed_key = 0;
    int64_t temp_slab_type;
    bool w_lock = false;
    while (! this->kv_list.empty()) {
      temp_hashed_key = this->kv_list.front();
      temp_slab_type  = this->kv_store[temp_hashed_key].value_length_type;
      if (new_block_size + WriteBuffer::slab_size[temp_slab_type] >
          this->page_per_block*1024*1024) {
        break;
      } else {
        this->kv_list.pop();
        this->blocks[latest_block_id].push_back(this->kv_store[temp_hashed_key]);
        this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[temp_slab_type]);
        new_block_size += WriteBuffer::slab_size[temp_slab_type];
      }
    }
    auto flush_func = std::bind(WriteBuffer::flush, latest_block_id);
    flush_thread_pool->schedule(flush_func);
  }
  while(flush_thread_pool->active() > 0 || flush_thread_pool->pending() > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  return 0;
}

int64_t WriteBuffer::close() {
  for (auto &i : this->kv_store) {
    delete[] i.second.value;
  }
  this->kv_store.clear();
  this->blocks.clear();
  while(!this->kv_list.empty())
    this->kv_list.pop();
  delete this->flush_thread_pool;
  return 0;
}

int64_t WriteBuffer::read(const std::string& key, char *value) {
  wbuffer_readLock read_lock(this->wbuffer_lock);
  size_t hashed_key  = std::hash<std::string>()(key);
  auto target = this->kv_store.find(hashed_key);
  if (target != this->kv_store.end()) {
    memcpy(value, target->second.value, target->second.value_length);
//  LOG(INFO) << "WRITE BUFFER CACHE HIT: " << key;
    return target->second.value_length;
  } else {
    return 0;
  }
}

} /* namespace cap */
