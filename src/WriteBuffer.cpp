/*
 * writebuffers.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "WriteBuffer.h"

namespace cap {

const int32_t WriteBuffer::slab_size[11] = {1*1024, 2*1024, 4*1024, 8*1024, 16*1024,
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


int32_t WriteBuffer::initial(const int32_t page_size,
                            const int32_t page_per_block,
                            const int32_t page_num) {
    this->page_size      = page_size;
    this->page_per_block = page_per_block;
    this->page_num       = page_num;
    this->max_page_num   = int32_t(1.2 * page_num);
    this->initial_flag   = 0;
    this->flush_thread_num = 6;
    std::string temp_num;
    leveldb::Status s = CentraIndex::singleton().db->Get(leveldb::ReadOptions(), "FILE_NUM", &temp_num);
    if (s.ok()) {
        this->latest_block_id = std::stoi(temp_num);
    } else {
        this->latest_block_id = 0;
    }

    this->flush_thread_pool = new boost::threadpool::pool(flush_thread_num);
//    this->latest_block_id= 0;
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    for (int32_t i=0; i<11; ++i) {
        this->page[i] = new boost::pool<>(WriteBuffer::slab_size[i]);
    }
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::write(const std::string& key, const char *value, int32_t length) {
    if (length > WriteBuffer::slab_size[10]) {
        return 1;
    }
    while (this->total_size > this->max_page_num * 1024*1024) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    size_t hashed_key  = std::hash<std::string>()(key);

    auto existed_key = this->kv_store.find(hashed_key);
    bool existence = false;

    if (existed_key != this->kv_store.end()) {
        return 1;
        existence = true;
        int32_t length_type = this->kv_store[hashed_key].value_length_type;
        this->page[length_type]->free(this->kv_store[hashed_key].value);
        //this->kv_store.erase(hashed_key);
        this->total_size.fetch_sub(WriteBuffer::slab_size[length_type]);
        this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[length_type]);
    }

    this->kv_store[hashed_key].key = key;
    this->kv_store[hashed_key].value_length = length;
    int32_t target_slab_type = std::ceil(std::log2(length/1024.0));
    //if the computed target_slab_type is less than 0
    //use the smallest slab type: 2^0 * 1024 (1KB)
    if (target_slab_type < 0) {
        target_slab_type = 0;
    }
    this->kv_store[hashed_key].value_length_type = target_slab_type;
    this->kv_store[hashed_key].value = (char*)page[target_slab_type]->malloc();
    memcpy(this->kv_store[hashed_key].value, value, length);

    if (existence == false) {
        this->kv_list.push(hashed_key);
    }
    this->total_size.fetch_add(WriteBuffer::slab_size[target_slab_type]);
    this->total_size_without_block.fetch_add(WriteBuffer::slab_size[target_slab_type]);

    if (this->total_size_without_block > (this->page_per_block*1024*1024)) { // 1 page 1 MB
        ++latest_block_id;
        int32_t new_block_size = 0;
        size_t temp_hashed_key = 0;
        int32_t temp_slab_type;
        while (true ) {
            temp_hashed_key = this->kv_list.front();
            temp_slab_type  = this->kv_store[temp_hashed_key].value_length_type;
            if (new_block_size + WriteBuffer::slab_size[temp_slab_type] >
                this->page_per_block*1024*1024) {
                break;
            } else {
                this->kv_list.pop();
                this->blocks[latest_block_id].push_back(this->kv_store[temp_hashed_key]);
                this->total_size_without_block.fetch_sub(WriteBuffer::slab_size[temp_slab_type]);
                this->kv_store.erase(temp_hashed_key);
                new_block_size += WriteBuffer::slab_size[temp_slab_type];
            }
        }
        auto flush_func = std::bind(WriteBuffer::flush, latest_block_id);
        flush_thread_pool->schedule(flush_func);
        CentraIndex::singleton().db->Put(leveldb::WriteOptions(),
                "FILE_NUM",
                std::to_string(latest_block_id).c_str());
    }
    return WRITEBUFFER_SUCCESS;
}

void WriteBuffer::flush(int32_t block_id) {

    int32_t flushed_size = 0;
    int32_t key_num = 0;
    std::string string_block_id = std::to_string(block_id);
    std::string fout_name = WriteBuffer::singleton().ssd_path
                    + "/"
                    + string_block_id;
    std::ofstream fout(fout_name);
    int32_t offset = 0;
    IndexInfo new_record;
    new_record.block_id = block_id;

    std::string a;
    for (auto i : WriteBuffer::singleton().blocks[block_id]) {
        a.clear();
        flushed_size += WriteBuffer::slab_size[i.value_length_type];
        ++key_num;
        //******
        fout.write(i.value, i.value_length);
        new_record.offset = offset;

        a.append(std::to_string(block_id));
        a.append("#");
        a.append(std::to_string(offset));
        CentraIndex::singleton().db->Put(leveldb::WriteOptions(), i.key, a);
        //*****
        offset += i.value_length;
        WriteBuffer::singleton().page[i.value_length_type]->free(i.value);

        std::string t;
        CentraIndex::singleton().db->Get(leveldb::ReadOptions(), i.key, &t);
        std::cout << t << "\n";
    }
    fout.close();

    WriteBuffer::singleton().blocks.erase(block_id);
    WriteBuffer::singleton().total_size.fetch_sub(flushed_size);
}


int32_t WriteBuffer::read(const std::string& key, char *value) {
    size_t hashed_key  = std::hash<std::string>()(key);
    auto target = this->kv_store.find(hashed_key);
    if (target != this->kv_store.end()) {
        memcpy(value, target->second.value, target->second.value_length);
        return target->second.value_length;
    } else {
        return 0;
    }
}

} /* namespace cap */
