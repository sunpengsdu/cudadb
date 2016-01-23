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
    name="";
    db=NULL;

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
    this->options.create_if_missing = true;
    this->options.block_cache = leveldb::NewLRUCache(128*1024*1024);
    this->options.write_buffer_size = 128*1024*1024;
    this->options.block_size = 64*1024;
    leveldb::Status status = leveldb::DB::Open(options, name.c_str(), &db);
    CHECK_EQ(status.ok(), true);
    return 0;
}

} /* namespace cap */



