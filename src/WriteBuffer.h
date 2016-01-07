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

#include <glog/logging.h>

namespace cap {

class WriteBuffer {
public:
    WriteBuffer();
    virtual ~WriteBuffer();

    int32_t page_size;
    int32_t page_per_block;
    int32_t page_num;
    int32_t block_num;
    int32_t initial_flag;

    char **pages;
    std::vector<std::vector<char*>> buffer;

    int32_t initial(const int32_t page_size,
                const int32_t page_per_block,
                const int32_t page_num);
    int32_t allocate_memory();

    int32_t write(const std::string& key, const char *value, int32_t length);
    int32_t read(const std::string& key, char *value);
};

} /* namespace cap */

#endif /* WRITEBUFFERS_H_ */
