/*
 * GpuCache.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_GPUCACHE_H_
#define SUNPENG_CAP_GPUCACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

namespace cap {

class GpuCache {
public:
    GpuCache();
    virtual ~GpuCache();

    int32_t page_size;
    int32_t page_num;
    char**  pages;
};

} /* namespace cap */

#endif /* GPUCACHE_H_ */
