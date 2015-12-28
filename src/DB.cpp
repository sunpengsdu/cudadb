//============================================================================
// Name        : DB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "./DB.h"

namespace cudadb {

//! DB constructor
DB::DB() {
    CHECK_EQ(cuInit(0), CUDA_SUCCESS);
    initial_para("./cudadb.yaml");
    get_device_num(&device_num);
}

//! DB constructor
/*!
  \param config_file configuration file path and name
*/
DB::DB(const std::string& config_file) {
    CHECK_EQ(cuInit(0), CUDA_SUCCESS);
    initial_para(config_file);
    get_device_num(&device_num);
}

int32_t DB::get_device_num(int32_t *num) {
    CHECK_EQ(cuDeviceGetCount(num), CUDA_SUCCESS);
    LOG(INFO) << "Find " << *num << " GPUs";
    return 0;
}

//! Initial parameter in the DB
/*!
  \param config_file configuration file path and name
  \return 0 if success
*/
int32_t DB::initial_para(const std::string& config_file) {
	//The default parameters
	page_size         = 1*1024*1024;    //1 MB
	cache_ram_size    = 1024;           //1024 pages
	cahe_gpu_size     = 1024;           //1024 pages
	block_size        = 16;             //16 pages
	buffer_ram_size   = 64;             //64 blocks, 1024 pages
	this->config_file = config_file;
	this->db_name     = "NULL";

	//check the existence of the yaml configuration file
	boost::filesystem::path db_config_path(config_file);
	if (boost::filesystem::exists(db_config_path) == false) {
		LOG(FATAL) << "The configure file "
				   << config_file
				   << " doesn't exist\n";
	}
	LOG(INFO) << "Check the configure file: pass";

	//load the configuration file and the parameters
	yaml_config = YAML::LoadFile(config_file);

	//load page_size
	if (yaml_config["page_size"]) {
		page_size = yaml_config["page_size"].as<int>()*1024;
		LOG(INFO) << "Find page_size in the configuration file";
	}
	LOG(INFO) << "Page Size: "
			  << page_size/1024
	          << " KB";

	//load cache_ram_size
	if (yaml_config["cache_ram_size"]) {
		page_size = yaml_config["cache_ram_size"].as<int>();
		LOG(INFO) << "Find cache_ram_size in the configuration file";
	}
	LOG(INFO) << "Cache Ram Size: "
			  << cache_ram_size
	          << " pages";

	//load cahe_gpu_size
	if (yaml_config["cahe_gpu_size"]) {
		page_size = yaml_config["cahe_gpu_size"].as<int>();
		LOG(INFO) << "Find cahe_gpu_size in the configuration file";
	}
	LOG(INFO) << "Cahe Gpu Size: "
			  << cahe_gpu_size
	          << " pages";

	//load block_size
	    if (yaml_config["block_size"]) {
	        page_size = yaml_config["block_size"].as<int>();
	        LOG(INFO) << "Find block_size in the configuration file";
	    }
	    LOG(INFO) << "Block Size: "
	              << block_size
	              << " pages";

	//load buffer_ram_size
	if (yaml_config["buffer_ram_size"]) {
		page_size = yaml_config["buffer_ram_size"].as<int>();
		LOG(INFO) << "Find buffer_ram_size in the configuration file";
	}
	LOG(INFO) << "Buffer Ram Size: "
			  << buffer_ram_size
	          << " blocks";

	return 0;
}

DB::~DB() {
}

//! Create a new DB
int32_t DB::create_db() {
    LOG(INFO) << "Create a new CudaDB: "
              << this->db_name;
    char* pages[10];
    db_malloc_gpu(1024, 1024, pages);
    while(1);
    return 0;
}

//! Load an existed DB
int32_t DB::load_db() {
    LOG(INFO) << "Load an existed CudaDB: "
              << this->db_name;
    return 0;
}

//! Open a DB, if it exists, load it; otherwise, create a DB with the name
/*!
    \param db_name the name of the db to open
    \return 0 if success
 */
int32_t DB::open(const std::string& db_name) {
    this->db_name = db_name;
    if (db_exist(db_name)) {
        load_db();
    } else {
        create_db();
    }
    return 0;
}

//! check the existence of a DB
/*!
    \param db_name the name of the db to check
    \return true if exists; otherwise false
 */
bool DB::db_exist(const std::string& db_name) {
    boost::filesystem::path db_file_path = boost::filesystem::path(db_name);
    if (boost::filesystem::exists(db_file_path)) {
        LOG(INFO) << "The DB file "
                  << db_name
                  << " exists";
        return true;
    } else {
        LOG(INFO) << "The db file "
                  << db_name
                  << " does not exists";
        return false;
    }
}

}
