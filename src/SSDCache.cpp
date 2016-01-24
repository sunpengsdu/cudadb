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
    thread_pool = new boost::threadpool::pool(1);
}

SSDCache::~SSDCache() {
    // TODO Auto-generated destructor stub
}

void SSDCache::flush(int32_t block_id) {

    SSDCache::singleton().unsynced_block_lock.lock();
    SSDCache::singleton().unsynced_block_id.erase(block_id);
    SSDCache::singleton().unsynced_block_lock.unlock();
}

SSDCache& SSDCache::singleton() {
   static SSDCache ssd_cache;
   return ssd_cache;
}

int32_t SSDCache::new_block(int32_t block_id) {
    this->unsynced_block_lock.lock();

    this->unsynced_block_id[block_id] = block_id;
    block_num.fetch_add(1);
    auto temp_func = std::bind(SSDCache::flush, block_id);
    this->thread_pool->schedule(temp_func);

    this->unsynced_block_lock.unlock();

    return 0;
}

} /* namespace cap */
