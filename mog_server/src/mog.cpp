//============================================================================
// Name        : DB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "mog.h"

namespace cap {

//! DB constructor
mog::mog() {
  setup("mog.yaml");
}

//! DB constructor
/*!
  \param config_file configuration file path and name
*/
mog::mog(const std::string& config_file) {
  setup(config_file);
}

mog::~mog() { }

int64_t mog::setup(const std::string& config_file) {
  initial_para(config_file);
  return MOG_SUCCESS;
}

//! Initial parameter in the DB
/*!
  \param config_file configuration file path and name
  \return 0 if success
*/
int64_t mog::initial_para(const std::string& config_file) {
  //The default parameters
  page_size       = 1*1024*1024;    //1 MB
  cpu_page_num    = 1;           
  page_per_block  = 64;          
  SSD_page_num    = 1;
  HDD_page_num    = 1;
  buffer_page_num = 1;
  this->config_file = config_file;
  this->db_name     = "NULL";
  ssd_path = "./";
  hdd_path = "./";
  this->ready = false;

  //check the existence of the yaml configuration file
  boost::filesystem::path db_config_path(config_file);
  if (boost::filesystem::exists(db_config_path) == false) {
    LOG(FATAL) << "The configure file " << config_file << " doesn't exist\n";
  }
  LOG(INFO) << "Check the configure file: pass";
  //load the configuration file and the parameters
  YAML::Node yaml_config = YAML::LoadFile(config_file);
  LOG(INFO) << config_file;

  //load page_size
  //if (yaml_config["page_size"]) {
  //  page_size = yaml_config["page_size"].as<int>()*1024;
  // }
  // Set Pagesize to 1MB statically
  page_size = 1024*1024;
  LOG(INFO) << "Page Size: " << page_size/1024/1024 << " MB";
  //load cache_ram_size
  if (yaml_config["RAM_Cache_Size"]) {
    cpu_page_num = yaml_config["RAM_Cache_Size"].as<int>();
  } else {
    //default is 1024 MB, 1GB
    cpu_page_num = 1; 
  }
  cpu_page_num *= 1024;
  LOG(INFO) << "RAM Cache Size: " << cpu_page_num/1024 << " GB";
  //load SSD cache size
  if (yaml_config["SSD_Cache_Size"]) {
    SSD_page_num = yaml_config["SSD_Cache_Size"].as<int>();
  } else {
    SSD_page_num = 1;
  }
  SSD_page_num *= 1024;
  LOG(INFO) << "SSD Cache Size: " << SSD_page_num/1024  << " GB";
  //load HDD cache size
  if (yaml_config["HDD_Cache_Size"]) {
    HDD_page_num = yaml_config["HDD_Cache_Size"].as<int>();
  } else {
    HDD_page_num = 1;
  }
  HDD_page_num *= 1024;
  LOG(INFO) << "HDD Cache Size: " << HDD_page_num/1024  << " GB";
  //load block_size
  if (yaml_config["Block_Size"]) {
    page_per_block = yaml_config["Block_Size"].as<int>();
  } else {
    page_per_block = 64;
  }
  LOG(INFO) << "Block Size: " << page_per_block << " MB";
  //load buffer_ram_size
  if (yaml_config["Buffer_Size"]) {
    buffer_page_num = yaml_config["Buffer_Size"].as<int>();
  } else {
    buffer_page_num = 1;
  }
  buffer_page_num *= 1024;
  LOG(INFO) << "Buffer Size: " << buffer_page_num/1024  << " GB";
  if (yaml_config["ssd_path"]) {
    ssd_path = yaml_config["ssd_path"].as<std::string>();
  }
  LOG(INFO) << "ssd_path: "<< ssd_path;
  if (yaml_config["hdd_path"]) {
    hdd_path = yaml_config["hdd_path"].as<std::string>();
  }
  LOG(INFO) << "hdd_path: " << hdd_path;
  return MOG_SUCCESS;
}

int64_t mog::allocate_memory() {
  CHECK_EQ(CpuCache::singleton().initial(page_size, cpu_page_num), CPUCACHE_SUCCESS);
  LOG(INFO) << "---------> Initial CPU Cache Done";
  CHECK_EQ(CpuCache::singleton().allocate_memory(), CPUCACHE_SUCCESS);
  LOG(INFO) << "---------> Create CPU Cache Done";
  CHECK_EQ(WriteBuffer::singleton().initial(page_size, page_per_block, buffer_page_num), WRITEBUFFER_SUCCESS);
  WriteBuffer::singleton().ssd_path = this->ssd_path;
  WriteBuffer::singleton().hdd_path = this->hdd_path;
  CHECK_EQ(WriteBuffer::singleton().allocate_memory(), WRITEBUFFER_SUCCESS);
  LOG(INFO) << "---------> Create Write Buffer Done ";
  SSDCache::singleton().hdd_path = this->hdd_path;
  SSDCache::singleton().ssd_path = this->ssd_path;
  SSDCache::singleton().max_block_num = this->SSD_page_num/this->page_per_block;
  SSDCache::singleton().initial();
  LOG(INFO) << "---------> Create SSD Cache Done ";
  return MOG_SUCCESS;
}

//! Create a new DB
int64_t mog::create() {
  boost::filesystem::path db_file_path_ssd = boost::filesystem::path(ssd_path + "/" + db_name + ".mog");
  boost::filesystem::path path_ssd = boost::filesystem::path(ssd_path);
  boost::filesystem::path path_hdd = boost::filesystem::path(hdd_path);
  boost::filesystem::remove_all(path_ssd);
  boost::filesystem::create_directories(db_file_path_ssd);
  boost::filesystem::remove_all(path_hdd);
  boost::filesystem::create_directories(path_hdd);

  CentraIndex::singleton().setup(ssd_path + "/" + db_name + ".mog");
  int64_t initial_block_id = 0;
  CentraIndex::singleton().put("FILE_NUM", strlen("FILE_NUM"), (char*)&initial_block_id, sizeof(initial_block_id));

  allocate_memory();
  LOG(INFO) << "Create a new Mog: " << this->db_name;
  return MOG_SUCCESS;
}

//! Load an existed DB
int64_t mog::load() {
  boost::filesystem::path db_file_path_ssd = boost::filesystem::path(ssd_path + "/");
  boost::filesystem::path path_ssd = boost::filesystem::path(ssd_path);
  boost::filesystem::remove_all(path_ssd);
  boost::filesystem::create_directory(path_ssd);
  std::string db_file_path_ssd_c = ssd_path + "/";
  std::string db_file_path_hdd_c = hdd_path + "/" + db_name+ ".mog";
  char cmd[256];
  sprintf(cmd, "cp -r %s %s", db_file_path_hdd_c.c_str(), db_file_path_ssd_c.c_str());
  system(cmd);
  CentraIndex::singleton().load(ssd_path + "/" + db_name + ".mog");
  LOG(INFO) << "Load an existed Mog: " << this->db_name;
  allocate_memory();
  return MOG_SUCCESS;
}

//! Open a DB, if it exists, load it; otherwise, create a DB with the name
/*!
    \param db_name the name of the db to open
    \return 0 if success
*/

int64_t mog::open(const std::string& db_name) {
  this->db_name = db_name;
  boost::filesystem::path ssd_cache_path = boost::filesystem::path(ssd_path);
  boost::filesystem::remove_all(ssd_cache_path);
  boost::filesystem::path hdd_cache_path = boost::filesystem::path(hdd_path);
  boost::filesystem::remove_all(hdd_cache_path);
  create();
//  if (exist(db_name)) {
//    load();
//  } else {
//    create();
//  }
  this->ready = true;
  return MOG_SUCCESS;
}

//! check the existence of a DB
/*!
    \param db_name the name of the db to check
    \return true if exists; otherwise false
 */
bool mog::exist(const std::string& db_name) {
  boost::filesystem::path db_file_path =
    boost::filesystem::path(hdd_path + "/" + db_name + ".mog");

  if (boost::filesystem::exists(db_file_path)) {
    LOG(INFO) << "The DB file " << db_name << " exists";
    return true;
  } else {
    LOG(INFO) << "The db file " << db_name << " does not exists";
    return false;
  }
}

int64_t mog::sync() {
  WriteBuffer::singleton().sync();
  SSDCache::singleton().sync();
  return MOG_SUCCESS;
}

int64_t mog::close() {
  this->ready = false;
//sync();
  WriteBuffer::singleton().close();
  LOG(INFO) << "Clean WriteBuffer Data\n";
//boost::filesystem::path db_file_path_hdd =
//boost::filesystem::path(hdd_path + "/" + db_name + ".mog");
//std::string db_file_path_ssd_c = ssd_path + "/" + db_name+ ".mog";
//std::string db_file_path_hdd_c = hdd_path + "/";
//if (boost::filesystem::exists(db_file_path_hdd)) {
//  boost::filesystem::remove_all(db_file_path_hdd);
// }
//char cmd[256];
//sprintf(cmd, "cp -r %s %s", db_file_path_ssd_c.c_str(), db_file_path_hdd_c.c_str());
//system(cmd);
//LOG(INFO) << "Update Index in hdd\n";
  CpuCache::singleton().close();
  LOG(INFO) << "Clean CPU Cache Data\n";
  CentraIndex::singleton().close();
  LOG(INFO) << "Clean Index\n";
  SSDCache::singleton().close();
  LOG(INFO) << "Clean SSD Data\n";
  boost::filesystem::path ssd_cache_path = boost::filesystem::path(ssd_path);
  boost::filesystem::remove_all(ssd_cache_path);
  LOG(INFO) << "Clean SSD Folder\n";
 // boost::filesystem::path hdd_cache_path = boost::filesystem::path(hdd_path);
 // boost::filesystem::remove_all(hdd_cache_path);
 // LOG(INFO) << "Clean HDD Data\n";
  return MOG_SUCCESS;
}

int64_t mog::write(const std::string& key, const char *value, int64_t length) {
  if (WriteBuffer::singleton().write(key, value, length) == 0) {
    return MOG_SUCCESS;
  } else {
    return -1;
  }
}

int64_t mog::read_file(const std::string &path, char *value) {
  int64_t mog_read_length = 0;
  int64_t mog_insert_result = 0;
  boost::filesystem::path target_path = boost::filesystem::path(path);
  //std::string complete_path = boost::filesystem::system_complete(target_path).string();
  //Done in caffe core io.cpp
  std::string complete_path = path;
  size_t complete_path_hash = std::hash<std::string> {}(complete_path);
  std::string complete_path_hash_s = std::to_string(complete_path_hash);
  
  //mog is not ready
  if (!this->ready) {
    std::ifstream ifs(path);
    if (! ifs.is_open()) {
      LOG(INFO) << path;
      return 0;
    }
    ifs.seekg(0, ifs.end);
    int64_t length = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    if (length > 64*1024*1024) {
      ifs.close();
      return 0;
    }
    char *buffer = new char[length+1];
    ifs.read(buffer, length);
    ifs.close();
    memcpy(value, buffer, length);
    delete[] buffer;
    return length;
  }
  //mog is ready
  mog_read_length = this->read(complete_path_hash_s, value);
  //mog_read_length = this->read(complete_path, value);
  if (mog_read_length > 0) {
    return mog_read_length;
  }
  if (mog_read_length == 0) {
    // Mog has no such file, in this case, Mog would fetch it from DFS
    mog_insert_result = this->insert_file_with_copy(complete_path_hash_s, path, value);
    //mog_insert_result = this->insert_file_with_copy(complete_path, path, value);
  }
  if (mog_insert_result > 0) {
    return mog_insert_result;
  } else {
    return 0;
  }
}

int64_t mog::insert_file_with_copy(const std::string &key, const std::string &file_path, char *value) {
  std::ifstream ifs(file_path);
  if (! ifs.is_open()) {
    return -1;
  }

  ifs.seekg(0, ifs.end);
  int64_t length = ifs.tellg();
  ifs.seekg(0, ifs.beg);
  if (length > 64*1024*1024) {
    ifs.close();
    return 0;
  }
  char *buffer = new char[length+1];
  ifs.read(buffer, length);
  ifs.close();
  if (length <= 1024*1024 && this->total_size <= 0.95*1024*1024*this->HDD_page_num) {
    int64_t res = this->write(key, buffer, length);
    this->total_size.fetch_add(length);
  }
  memcpy(value, buffer, length);
  delete[] buffer;
  return length;
}

int64_t mog::insert_file(const std::string &key, const std::string &file_path) {
  std::ifstream ifs(file_path);
  if (! ifs.is_open()) {
    return -1;
  }

  ifs.seekg(0, ifs.end);
  int64_t length = ifs.tellg();
  ifs.seekg(0, ifs.beg);
  if (length > 1024*1024) {
    return -1;
  }
  char *buffer = new char[length+1];
  ifs.read(buffer, length);
  ifs.close();
  int64_t res = this->write(key, buffer, length);
  delete[] buffer;
  if (res == 0) {
    return length;
  } else {
    return -1;
  }
}

int64_t mog::read(const std::string& key, char *value) {
  int64_t length = 0;
  length = WriteBuffer::singleton().read(key,value);
  if (length > 0) {
    return length;
  } else {
    length = CpuCache::singleton().read(key, value);
    return length;
  }
}

}
