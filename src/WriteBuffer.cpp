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
    page_size      = 0;
    page_per_block = 0;
    max_page_per_block = 0;
    page_num       = 0;
    max_page_num   = 0;
    block_num      = 0;
    pages          = NULL;
    initial_flag   = -1;
}

WriteBuffer::~WriteBuffer() {
    // TODO Auto-generated destructor stub
}

int32_t WriteBuffer::initial(const int32_t page_size,
                            const int32_t page_per_block,
                            const int32_t page_num) {
    this->page_size      = page_size;
    this->page_per_block = page_per_block;
    this->max_page_per_block = int32_t(1.5 * page_per_block);
    this->page_num       = page_num;
    this->max_page_num   = int32_t(1.5 * page_num);
    this->block_num      = page_num / page_per_block;
    this->pages          = new char*[max_page_num];
    this->initial_flag   = 0;

    block.resize(this->block_num);
    used_page.clear();
    free_page.clear();

    LOG(INFO) << "Write buffer has " << this->block_num << " blocks";

    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    int32_t block_id = 0;
    for (int32_t page_id = 0; page_id < max_page_num; ++page_id) {
        pages[page_id] = (char *)malloc(page_size);
        memset(pages[page_id], 0, page_size);
        free_page.push_back(page_id);
        if (page_id / max_page_per_block < block_num &&
                page_id % max_page_per_block == 0) {
            block[block_id].block_id = block_id;
            block[block_id].used_page_id.clear();
            block[block_id].used_page_group_by_slab.clear();
            block[block_id].used_page_num = 0;
            ++block_id;
        }
    }
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::write(const std::string& key, const char *value, int32_t length) {
    size_t hashed_key  = std::hash<std::string>()(key);
    int32_t key_length = key.length();
    int32_t block_id   = hashed_key % block_num;
    int32_t total_size = sizeof(size_t) + sizeof(int32_t) * 2 + key.length() + length;

    //based on the total data size (i.e., hashed_key length + 2*sizeof(int32_t) + key length + value length)
    //choose the suited slab type
    //tager_slab_type is the index number of
    //const int32_t WriteBuffer::slab_size[11] = {1*1024, 2*1024, 4*1024, 8*1024, 16*1024,
    //                                            32*1024, 64*1024, 128*1024, 256*1024,
    //                                            512*1024, 1024*1024};
    int32_t target_slab_type = std::ceil(std::log2(total_size/1024.0));

    //if the computed target_slab_type is less than 0
    //use the smallest slab type: 2^0 * 1024 (1KB)
    if (target_slab_type < 0) {
        target_slab_type = 0;
    }

    //if the computed target_slab_type is large than 10 (1MB)
    //return 1;
    //it means that mog will not store the data
    if (target_slab_type > 10) {
        return 1;
    }

    //first check whether the block exists
    CHECK_EQ(block[block_id].block_id, block_id);

    //find the target page_id to write the data
    int32_t target_page_id = -1;

    //traverse all the suited page (based on the slab type)
    for (auto page_with_target_slab:block[block_id].used_page_group_by_slab[target_slab_type]) {
        //find a page with free slab
        //set this page_id as the target page_id to write the data in
        if (used_page[page_with_target_slab].free_slab_num > 0) {
            target_page_id = page_with_target_slab;
            break;
        }
    }

    //if we cannot find an existing page, and we still have free pages
    //apply for a new page, and initial it
    if (target_page_id == -1 && !free_page.empty()) {
        int32_t new_page = *(free_page.end() - 1);
        free_page.pop_back();
        used_page[new_page].block_id = block_id;
        used_page[new_page].max_slab_num = page_size / WriteBuffer::slab_size[target_slab_type];
        used_page[new_page].free_slab_num = used_page[new_page].max_slab_num;
        used_page[new_page].page_id = new_page;
        used_page[new_page].page_p  = pages[new_page];
        used_page[new_page].slab_size = target_slab_type;
        used_page[new_page].used_slab_num = 0;
        used_page[new_page].free_slab.clear();
        for (int32_t slab_id = 0; slab_id < used_page[new_page].max_slab_num; ++slab_id) {
            used_page[new_page].free_slab.push_back(slab_id);
        }

        block[block_id].used_page_num++;
        block[block_id].used_page_id.push_back(new_page);
        block[block_id].used_page_group_by_slab[target_slab_type].push_back(new_page);

        char* dest_slab_p = used_page[new_page].page_p;
        int32_t offset = *(used_page[new_page].free_slab.end() - 1) * WriteBuffer::slab_size[target_slab_type];
        dest_slab_p +=  offset;
        used_page[new_page].free_slab.pop_back();

        memcpy(dest_slab_p, &hashed_key, sizeof(size_t));
        dest_slab_p += sizeof(size_t);
        memcpy(dest_slab_p, &key_length, sizeof(int32_t));
        dest_slab_p += sizeof(int32_t);
        memcpy(dest_slab_p, &length, sizeof(int32_t));
        dest_slab_p += sizeof(int32_t);
        memcpy(dest_slab_p, key.c_str(), key_length);
        dest_slab_p += key_length;
        memcpy(dest_slab_p, value, length);

        used_page[new_page].free_slab_num--;
        used_page[new_page].used_slab_num++;

        LOG(INFO) << "Write: key->" << key
                  << " block->" << block_id
                  << " page->"  << new_page
                  << "@"        << offset / WriteBuffer::slab_size[target_slab_type];
    // if we find an existing page with free slabs
    // then we can just write the data into this page on suitable place
    // append the data in the page
    } else if (target_page_id != -1){
        char* dest_slab_p = used_page[target_page_id].page_p;
        int32_t offset = *(used_page[target_page_id].free_slab.end() - 1) * WriteBuffer::slab_size[target_slab_type];
        dest_slab_p +=  offset;
        used_page[target_page_id].free_slab.pop_back();

        memcpy(dest_slab_p, &hashed_key, sizeof(size_t));
        dest_slab_p += sizeof(size_t);
        memcpy(dest_slab_p, &key_length, sizeof(int32_t));
        dest_slab_p += sizeof(int32_t);
        memcpy(dest_slab_p, &length, sizeof(int32_t));
        dest_slab_p += sizeof(int32_t);
        memcpy(dest_slab_p, key.c_str(), key_length);
        dest_slab_p += key_length;
        memcpy(dest_slab_p, value, length);

        used_page[target_page_id].free_slab_num--;
        used_page[target_page_id].used_slab_num++;

        LOG(INFO) << "Write: key->" << key
                  << " block->" << block_id
                  << " page->"  << target_page_id
                  << "@"        << offset / WriteBuffer::slab_size[target_slab_type];

    // if cannot find an existing page and there is no free page to apply
    // we need to wait or drop this write operation
    } else {

        ///
        /// TODO
        ///
    }

    if(block[block_id].used_page_num < this->page_per_block) {


    } else if(block[block_id].used_page_num >= this->page_per_block
            && block[block_id].used_page_num < this->max_page_per_block) {

    } else {

    }
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::read(const std::string& key, char *value) {

    return WRITEBUFFER_SUCCESS;
}

} /* namespace cap */
