/*
 * writebuffers.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_WRITEBUFFERS_H_
#define SUNPENG_CAP_WRITEBUFFERS_H_

#define WRITEBUFFER_SUCCESS 0

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

#include <glog/logging.h>

struct MogBufferBlock {
    int32_t block_id;
    int32_t used_page_num;
    std::vector<int32_t> used_page_id;
    std::map<int32_t, std::vector<int32_t>> used_page_group_by_slab;
};

struct MogUsedBufferPage {
    char* page_p;
    int32_t page_id;
    int32_t block_id;
    int32_t slab_size;
    int32_t max_slab_num;
    int32_t used_slab_num;
    int32_t free_slab_num;
    std::vector<int32_t> free_slab;
};

namespace cap {

class WriteBuffer {
public:
    WriteBuffer();
    virtual ~WriteBuffer();

    int32_t page_size;
    int32_t page_per_block;
    int32_t max_page_per_block;
    int32_t page_num;
    int32_t max_page_num;
    int32_t block_num;
    int32_t initial_flag;
    static const int32_t slab_size[11];//KB

    std::map<std::int32_t, MogUsedBufferPage> used_page;
    std::vector<std::int32_t> free_page;
    std::vector<MogBufferBlock> block;

    char **pages;
    //std::vector<std::vector<char*>> buffer;

    int32_t initial(const int32_t page_size,
                const int32_t page_per_block,
                const int32_t page_num);
    int32_t allocate_memory();

    int32_t write(const std::string& key, const char *value, int32_t length);
    int32_t read(const std::string& key, char *value);
};

} /* namespace cap */

#endif /* WRITEBUFFERS_H_ */
