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

#include "./include/leveldb/c.h"
#include "./include/leveldb/options.h"
#include "./include/leveldb/status.h"
#include "./include/leveldb/iterator.h"
#include "./include/leveldb/cache.h"

struct leveldb_options_t {
    leveldb::Options rep;
};

namespace cap {

struct IndexInfo {
    int64_t block_id;
    int64_t offset;
    int64_t length;
    int64_t length_type;
};

class CentraIndex {
public:
    CentraIndex();
    virtual ~CentraIndex();

    static CentraIndex& singleton();
    int64_t setup(const std::string &name);
    int64_t load(const std::string &name);

    int64_t put(const char* key, int64_t key_length, const char* value, int64_t value_length);
    int64_t get(const char* key, int64_t key_length, char *value, int64_t buffer_length);
    int64_t close();


    leveldb_t* db;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;
    leveldb_writeoptions_t *woptions;
    leveldb_cache_t *cache;
    char *err;
    std::string name;
};

} /* namespace cap */

#endif /* SUNPENG_CAP_CENTRAINDEX_H_ */
