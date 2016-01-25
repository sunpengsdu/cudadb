/*
 * SSDCache.cpp
 *
 *  Created on: 24 Jan 2016
 *      Author: sunp
 */

#include "SSDCache.h"

namespace cap {

SSDCache::SSDCache() {
    // TODO Auto-generated constructor stub
    max_block_num = 0;
    block_num = 0;
    thread_pool = new boost::threadpool::pool(6);
    cached_block.reserve(max_block_num*2);
    unsynced_block_id.reserve(max_block_num*2);
}

SSDCache::~SSDCache() {
    // TODO Auto-generated destructor stub
}

void SSDCache::flush(int32_t block_id) {

    std::string src_name = SSDCache::singleton().ssd_path + "/" + std::to_string(block_id);
    std::string dst_name = SSDCache::singleton().dfs_path + "/" + std::to_string(block_id);

    boost::filesystem::copy_file(boost::filesystem::path(src_name),
            boost::filesystem::path(dst_name));

    ssd_cache_writeLock(SSDCache::singleton().ssd_cache_rwlock);
    //SSDCache::singleton().unsynced_block_lock.lock();
    int32_t target_file_size = SSDCache::singleton().unsynced_block_id[block_id];
    SSDCache::singleton().unsynced_block_id.erase(block_id);
   // SSDCache::singleton().unsynced_block_lock.unlock();

   // SSDCache::singleton().cached_block_lock.lock();
    SSDCache::singleton().cached_block_id.push_back(block_id);
    SSDCache::singleton().cached_block[block_id] = target_file_size;
   // SSDCache::singleton().cached_block_lock.unlock();

    LOG(INFO) << "FLUSH BLOCK "
            << block_id
            << " FROM SSD TO DFS";
}

SSDCache& SSDCache::singleton() {
   static SSDCache ssd_cache;
   return ssd_cache;
}

int32_t SSDCache::new_block(int32_t block_id) {

    ssd_cache_writeLock(this->ssd_cache_rwlock);
    //this->unsynced_block_lock.lock();
    boost::filesystem::path target_path(this->ssd_path + "/" + std::to_string(block_id));
    this->unsynced_block_id[block_id] = boost::filesystem::file_size(target_path);
    block_num.fetch_add(1);
    auto temp_func = std::bind(SSDCache::flush, block_id);
    this->thread_pool->schedule(temp_func);

   // this->unsynced_block_lock.unlock();

    return 0;
}

int32_t SSDCache::read(const std::string &key, const IndexInfo &key_info, char* value) {

    ssd_cache_readLock(this->ssd_cache_rwlock);


    std::string target_file = this->ssd_path + "/" + std::to_string(key_info.block_id);
    FILE *p_file = fopen(target_file.c_str(), "rb");
    if (p_file == NULL) {
        LOG(FATAL) << "CANNOT FIND FILE IN SSD";
    }

    fseek(p_file, key_info.offset, 0);
    fread((void*)value, 1, key_info.length, p_file);
    fclose(p_file);
    return key_info.length;
}

} /* namespace cap */
