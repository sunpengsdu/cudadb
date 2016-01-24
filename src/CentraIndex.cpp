/*
 * CentraIndex.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: sunp
 */

#include "CentraIndex.h"

namespace cap {

CentraIndex::CentraIndex() {
    // TODO Auto-generated constructor stub
    name = "";
    db   = NULL;
    err  = NULL;
}

CentraIndex::~CentraIndex() {
    // TODO Auto-generated destructor stub
}

CentraIndex& CentraIndex::singleton() {
   static CentraIndex centra_index;
   return centra_index;
}

int32_t CentraIndex::setup(const std::string &name) {
    this->name = name;
    this->options = leveldb_options_create();
    this->cache = leveldb_cache_create_lru(128*1024*1024);
    this->woptions = leveldb_writeoptions_create();
    this->roptions = leveldb_readoptions_create();

    leveldb_options_set_create_if_missing(this->options, 1);
    leveldb_options_set_write_buffer_size(this->options, 128*1024*1024);
    leveldb_options_set_block_size(this->options, 64*1024);
    leveldb_options_set_cache(this->options, this->cache);
    this->err = NULL;

    this->db = leveldb_open(this->options, name.c_str(), &this->err);
    if (this->err != NULL) {
        LOG(FATAL) << "CANNOT CREATE CentralIndex in " << name;
    }
    leveldb_free(this->err);
    return 0;
}

int32_t CentraIndex::put(const char* key, int32_t key_length,  const char* value, int32_t value_length) {
    char *put_err = NULL;
   // LOG(INFO) << key << "->" << value;
    leveldb_put(this->db,
            this->woptions,
            key,
            key_length,
            value,
            value_length,
            &put_err);
    if (put_err != NULL) {
        LOG(WARNING) << "CentraIndex Put Error\n";
        leveldb_free(put_err);
        return -1;
    } else {
        return 0;
    }
}
int32_t CentraIndex::get(const char* key, int32_t key_length, char *value, int32_t buffer_length) {
    char *get_err = NULL;
    char *temp_result;
    size_t value_length;
    temp_result = leveldb_get(this->db,
            this->roptions,
            key,
            key_length,
            &value_length,
            &get_err);
    if (get_err != NULL) {
        LOG(WARNING) << "CentraIndex Get Error\n";
        leveldb_free(get_err);
        return 0;
    } else {
        if (temp_result != NULL && buffer_length >= value_length) {
            memcpy(value, temp_result, value_length);
            leveldb_free(temp_result);
            leveldb_free(get_err);
            return value_length;
        } else if (temp_result == NULL) {
            leveldb_free(get_err);
            return 0;
        } else {
            leveldb_free(temp_result);
            leveldb_free(get_err);
            return 0;
        }
    }
}

} /* namespace cap */



