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
    this->flush_thread_num = 6;
}

WriteBuffer::~WriteBuffer() {
    // TODO Auto-generated destructor stub
}

WriteBuffer& WriteBuffer::singleton() {
   static WriteBuffer write_buffer;
   return write_buffer;
}
int64_t WriteBuffer::initial(const int64_t page_size,
    const int64_t page_per_block,
    const int64_t page_num) {

    this->page_size      = page_size;
    this->page_per_block = page_per_block;
    this->page_num       = page_num;
    this->max_page_num   = int64_t(1.2 * page_num);
    this->initial_flag   = 0;
    this->flush_thread_num = 6;
    int64_t temp_num = 0;
    int64_t tempt_read_len;
    tempt_read_len = CentraIndex::singleton().get("FILE_NUM",
        strlen("FILE_NUM"),
        (char*)&temp_num,
        sizeof(temp_num));

    if (tempt_read_len != 0) {
        this->latest_block_id = temp_num;
    } else {
        this->latest_block_id = 0;
    }
    this->flush_thread_pool = new boost::threadpool::pool(flush_thread_num);
    this->kv_store.reserve(this->max_page_num*1024*1.2);
    this->blocks.reserve(this->max_page_num*1.2);
//    this->latest_block_id= 0;
    return WRITEBUFFER_SUCCESS;
}

int64_t WriteBuffer::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    for (int64_t i=0; i<11; ++i) {
        this->page[i] = new boost::pool<>(WriteBuffer::slab_size[i]);
    }
    return WRITEBUFFER_SUCCESS;
}

int64_t WriteBuffer::write(const std::string& key, const char *value, int64_t length) {
    if (length > WriteBuffer::slab_size[10]) {
        return -1;
    }

    IndexInfo key_centra_index;
    int64_t index_read_length = 0;
    //find in central index
    index_read_length = CentraIndex::singleton().get(key.c_str(),
        key.length(),
        (char*)&key_centra_index,
        sizeof(IndexInfo));

    if (index_read_length == sizeof(IndexInfo)) {
//        LOG(INFO) << "KEY EXIST " << key;
        return -1;
    }

//    LOG(INFO) << "WriteBuffer Size: " << this->total_size
//            <<" Max Size: " << this->max_page_num * this->page_size;
    while (this->total_size > this->max_page_num * this->page_size) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    while (SSDCache::singleton().block_num >= SSDCache::singleton().max_block_num) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    wbuffer_writeLock write_lock(this->wbuffer_lock);
    size_t hashed_key  = std::hash<std::string>()(key);
    auto existed_key = this->kv_store.find(hashed_key);
    bool existence = false;
    if (existed_key != this->kv_store.end()) {
        return -1;
        //***********************
        existence = true;
        int64_t length_type = this->kv_store[hashed_key].value_length_type;
        this->page[length_type]->free(this->kv_store[hashed_key].value);
        //this->kv_store.erase(hashed_key);
        this->total_size.fetch_sub(WriteBuffer::slab_size[length_type]);
        this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[length_type]);
    }
//    this->kv_store[hashed_key].w_lock = 1;
//    this->kv_store[hashed_key].r_lock = 0;
    this->kv_store[hashed_key].key  = key;
    this->kv_store[hashed_key].value_length = length;
    int64_t target_slab_type = std::ceil(std::log2(length/1024.0));
    //if the computed target_slab_type is less than 0
    //use the smallest slab type: 2^0 * 1024 (1KB)
    if (target_slab_type < 0) {
        target_slab_type = 0;
    }
    this->kv_store[hashed_key].value_length_type = target_slab_type;
    this->kv_store[hashed_key].value = (char*)page[target_slab_type]->malloc();
    memcpy(this->kv_store[hashed_key].value, value, length);
//    this->kv_store[hashed_key].w_lock = 0;

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
        while (true ) {
            temp_hashed_key = this->kv_list.front();
//            this->kv_store[hashed_key].w_lock = 1;
//            while(! __sync_bool_compare_and_swap(&this->kv_store[hashed_key].r_lock, 0, 0));

            temp_slab_type  = this->kv_store[temp_hashed_key].value_length_type;
            if (new_block_size + WriteBuffer::slab_size[temp_slab_type] >
                this->page_per_block*1024*1024) {
                break;
            } else {
                this->kv_list.pop();
                this->blocks[latest_block_id].push_back(this->kv_store[temp_hashed_key]);
                this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[temp_slab_type]);
                //**********************
                //                this->kv_store.erase(temp_hashed_key);
                //**********************
                new_block_size += WriteBuffer::slab_size[temp_slab_type];
            }
        }
        auto flush_func = std::bind(WriteBuffer::flush, latest_block_id);
        flush_thread_pool->schedule(flush_func);
        CentraIndex::singleton().put("FILE_NUM",
                strlen("FILE_NUM"),
                (char*)&latest_block_id,
                sizeof(latest_block_id));
    }
    return WRITEBUFFER_SUCCESS;
}

void WriteBuffer::flush(int64_t block_id) {

    int64_t flushed_size = 0;
    int64_t key_num = 0;
    std::string string_block_id = std::to_string(block_id);
    std::string fout_name = WriteBuffer::singleton().ssd_path + "/" + string_block_id;
    std::ofstream fout(fout_name);
    int64_t offset = 0;
    IndexInfo new_record;
    new_record.block_id = block_id;

    wbuffer_writeLock write_lock(WriteBuffer::singleton().wbuffer_lock);

    for (auto i : WriteBuffer::singleton().blocks[block_id]) {
        flushed_size += WriteBuffer::slab_size[i.value_length_type];
        ++key_num;
        //******
        fout.write(i.value, i.value_length);
        new_record.offset = offset;
        new_record.length = i.value_length;
        new_record.length_type = i.value_length_type;

        CentraIndex::singleton().put(i.key.c_str(),
                i.key.length(),
                (char*)&new_record,
                sizeof(new_record));
        //*****
        offset += i.value_length;
        WriteBuffer::singleton().page[i.value_length_type]->free(i.value);

        WriteBuffer::singleton().kv_store.erase(std::hash<std::string>()(i.key));
//        LOG(INFO) << "<<< Erase Key: " << (i.key) << " From Write Buffer";
//
//        IndexInfo t;
//        CentraIndex::singleton().get(i.key.c_str(),
//                i.key.length(),
//                (char*)&t,
//                sizeof(t));
//        std::cout << i.key << "->" << t.block_id << "#" << t.length << "$" << i.value_length_type << "@" << t.offset << "\n";
    }
    fout.close();

    SSDCache::singleton().new_block(block_id);
    WriteBuffer::singleton().blocks.erase(block_id);
    WriteBuffer::singleton().total_size.fetch_sub(flushed_size);
//    LOG(INFO) << "FLUSH BLOCK "
//              << block_id
//              << " FROM BUFFER TO SSD";
//    LOG(INFO) << "WriteBuffer Size: " 
//              << WriteBuffer::singleton().total_size
//              <<" Max Size: "
//              << WriteBuffer::singleton().max_page_num * WriteBuffer::singleton().page_size;
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
                //**********************
                //                this->kv_store.erase(temp_hashed_key);
                //**********************
                new_block_size += WriteBuffer::slab_size[temp_slab_type];
            }
        }
        auto flush_func = std::bind(WriteBuffer::flush, latest_block_id);
        flush_thread_pool->schedule(flush_func);
        CentraIndex::singleton().put("FILE_NUM",
            strlen("FILE_NUM"),
            (char*)&latest_block_id,
            sizeof(latest_block_id));
    }

    while(flush_thread_pool->active() > 0 || flush_thread_pool->pending() > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    return 0;
}

int64_t WriteBuffer::close() {
    this->kv_store.clear();
    return 0;
}

int64_t WriteBuffer::read(const std::string& key, char *value) {

    wbuffer_readLock read_lock(this->wbuffer_lock);

    size_t hashed_key  = std::hash<std::string>()(key);
    auto target = this->kv_store.find(hashed_key);
    if (target != this->kv_store.end()) {
//        if(target->second.w_lock == 1) {
//            return 0;
//        }
//        __sync_fetch_and_add(&target->second.r_lock, 1);
        memcpy(value, target->second.value, target->second.value_length);
//        __sync_fetch_and_add(&target->second.r_lock, -1);

        LOG(INFO) << "WRITE BUFFER CACHE HIT: " << key;
        return target->second.value_length;
    } else {
        return 0;
    }
}

} /* namespace cap */
