/*
 * writebuffers.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "WriteBuffer.h"

namespace cap {

WriteBuffer::WriteBuffer() {
    page_size      = 0;
    page_per_block = 0;
    page_num       = 0;
    block_num      = 0;
    pages          = NULL;
    initial_flag   = -1;

}

WriteBuffer::~WriteBuffer() {
    // TODO Auto-generated destructor stub
}

int32_t WriteBuffer::write(const std::string& key, const char *value, int32_t length) {
    int32_t block_id = std::hash<std::string>()(key) % block_num;
    std::cout << block_id << "###\n";
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::initial(const int32_t page_size,
                            const int32_t page_per_block,
                            const int32_t page_num) {
    this->page_size      = page_size;
    this->page_per_block = page_per_block;
    this->page_num       = page_num;
    this->block_num      = page_num / page_per_block;
    this->pages          = new char*[page_num];
    this->initial_flag   = 0;
    LOG(INFO) << "Write buffer has " << this->block_num << " blocks";

    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    for (int32_t page_id = 0; page_id < page_num; ++page_id) {
        pages[page_id] = (char *)malloc(page_size);
        memset(pages[page_id], 0, page_size);
        if (page_id / page_per_block < block_num) {
            if (page_id % page_per_block == 0) {
                std::vector<char*> block;
                buffer.push_back(block);
            }
            (buffer.end() - 1)->push_back(pages[page_id]);
        }
    }
    return WRITEBUFFER_SUCCESS;
}

int32_t WriteBuffer::read(const std::string& key, char *value) {

    return WRITEBUFFER_SUCCESS;
}

} /* namespace cap */
