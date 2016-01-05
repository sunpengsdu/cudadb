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
    setup("./mog.yaml");
}

//! DB constructor
/*!
  \param config_file configuration file path and name
*/
mog::mog(const std::string& config_file) {
    setup(config_file);
}

mog::~mog() {
}

int32_t mog::setup(const std::string& config_file) {
    CHECK_EQ(cuInit(0), CUDA_SUCCESS);
    get_device_num(&device_num);
    CHECK_NE(device_num, 0);
    initial_para(config_file);
    return 0;
}

int32_t mog::mog_malloc_cpu(int32_t page_size, char** pages, int32_t page_id) {
    char *page = (char *)malloc(page_size);
    pages[page_id] = page;
    return 0;
}

int32_t mog::get_device_num(int32_t *num) {
    CHECK_EQ(cuDeviceGetCount(num), CUDA_SUCCESS);
    CHECK_NE(device_num, 0);
    LOG(INFO) << "Find " << *num << " GPUs";
    return 0;
}

//! Initial parameter in the DB
/*!
  \param config_file configuration file path and name
  \return 0 if success
*/
int32_t mog::initial_para(const std::string& config_file) {
	//The default parameters
	page_size       = 1*1024*1024;    //1 MB
	cpu_page_num    = 1024;           //1024 pages
	gpu_page_num    = 1024;           //1024 pages
	block_page_num  = 16;             //16 pages
	buffer_page_num = 1024;
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
	}
	LOG(INFO) << "Page Size: "
			  << page_size/1024
	          << " KB";

	//load cache_ram_size
	if (yaml_config["cpu_page_num"]) {
	    cpu_page_num = yaml_config["cpu_page_num"].as<int>();
	}
	LOG(INFO) << "Cache CPU Size: "
			  << cpu_page_num
	          << " pages";

	//load cahe_gpu_size
	if (yaml_config["gpu_page_num"]) {
	    gpu_page_num = yaml_config["gpu_page_num"].as<int>();
	}
	LOG(INFO) << "Cache Gpu Size: "
			  << gpu_page_num
	          << " pages";

	//load block_size
	    if (yaml_config["block_page_num"]) {
	        block_page_num = yaml_config["block_page_num"].as<int>();
	    }
	    LOG(INFO) << "Block Size: "
	              << block_page_num
	              << " pages";

	//load buffer_ram_size
	if (yaml_config["buffer_page_num"]) {
	    buffer_page_num = yaml_config["buffer_page_num"].as<int>();
	}
	LOG(INFO) << "Buffer Ram Size: "
			  << buffer_page_num
	          << " pages";

    //load buffer_ram_size
    if (yaml_config["devices"]) {
        std::string used_devices = yaml_config["devices"].as<std::string>();
        std::string parsed_device("");
        for (auto n:used_devices) {
            if (n != ',') {
                parsed_device += n;
            } else {
                if (n == ',' && parsed_device != "") {
                    devices.push_back(std::stoi(parsed_device));
                    CHECK_EQ(std::stoi(parsed_device) < device_num, true);
                    LOG(INFO) << "Used Devices: " << std::stoi(parsed_device);
                    parsed_device.clear();
                }
            }
        }
        if (parsed_device != "") {
            devices.push_back(std::stoi(parsed_device));
            CHECK_EQ(std::stoi(parsed_device) < device_num, true);
            LOG(INFO) << "Used Devices: " << std::stoi(parsed_device);
        }

    } else {
        devices.push_back(0);
        LOG(INFO) << "Used Devices: " << 0;
    }
	return 0;
}

int32_t mog::allocate_memory() {
    for (auto device_id : devices) {
        gpu_caches[device_id] = GpuCache();
        gpu_caches[device_id].page_num   = gpu_page_num;
        gpu_caches[device_id].page_size  = page_size;
        gpu_caches[device_id].pages      = new char*[gpu_page_num];
        for (int32_t page_id = 0; page_id < gpu_page_num; ++page_id) {
            mog_malloc_gpu(device_id,
                        page_size,
                        gpu_caches[device_id].pages,
                        page_id);
        }
    }
    cpu_caches.page_num   = cpu_page_num;
    cpu_caches.page_size  = page_size;
    cpu_caches.pages      = new char*[cpu_page_num];

    for (int32_t page_id = 0; page_id < cpu_page_num; ++page_id) {
        mog_malloc_cpu(page_size,
                    cpu_caches.pages,
                    page_id);
        memset(cpu_caches.pages[page_id], '!', page_size);
    }
    return 0;
}

//! Create a new DB
int32_t mog::create_db() {
    allocate_memory();
    mog_memcpy_cpu_to_gpu(0, gpu_caches[0].pages[1], cpu_caches.pages[0], page_size);

    while(1) {
    mog_vectorAdd(0, gpu_caches[0].pages[1], gpu_caches[0].pages[0], gpu_caches[0].pages[1], 1024*1024); }
    mog_memcpy_gpu_to_cpu(0, cpu_caches.pages[0], gpu_caches[0].pages[1], page_size);
    std::cout << "###"<< cpu_caches.pages[0][0]<<"#######\n";

    LOG(INFO) << "Create a new CudaDB: "
              << this->db_name;
    return 0;
}

//! Load an existed DB
int32_t mog::load_db() {
    allocate_memory();
    LOG(INFO) << "Load an existed CudaDB: "
              << this->db_name;
    return 0;
}

//! Open a DB, if it exists, load it; otherwise, create a DB with the name
/*!
    \param db_name the name of the db to open
    \return 0 if success
 */
int32_t mog::open(const std::string& db_name) {
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
bool mog::db_exist(const std::string& db_name) {
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
