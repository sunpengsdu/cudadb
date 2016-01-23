/*
 * CentraIndex.h
 *
 *  Created on: 23 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_CENTRAINDEX_H_
#define SUNPENG_CAP_CENTRAINDEX_H_

#include <vector>
#include <string>
#include <iostream>
#include <glog/logging.h>

#include "./include/leveldb/db.h"
#include "./include/leveldb/cache.h"

namespace cap {

struct IndexInfo {
    int32_t block_id;
    int32_t offset;
};

class CentraIndex {
public:
    CentraIndex();
    virtual ~CentraIndex();

    static CentraIndex& singleton();
    int32_t setup(const std::string &name);

    leveldb::DB* db;
    leveldb::Options options;
    std::string name;
};

} /* namespace cap */

#endif /* SUNPENG_CAP_CENTRAINDEX_H_ */
