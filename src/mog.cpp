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
    return MOG_SUCCESS;
}

int32_t mog::get_device_num(int32_t *num) {
    CHECK_EQ(cuDeviceGetCount(num), CUDA_SUCCESS);
    CHECK_NE(device_num, 0);
    LOG(INFO) << "Find " << *num << " GPUs";
    return MOG_SUCCESS;
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
	page_per_block  = 16;             //16 pages
	SSD_page_num    = 1024;
	buffer_page_num = 1024;
	this->config_file = config_file;
	this->db_name     = "NULL";
	ssd_path = "~";
	dfs_path = "~";

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

    if (yaml_config["SSD_page_num"]) {
        SSD_page_num = yaml_config["SSD_page_num"].as<int>();
    }
    LOG(INFO) << "SSD Page Num: "
              << SSD_page_num
              << " pages";

	//load block_size
	    if (yaml_config["page_per_block"]) {
	        page_per_block = yaml_config["page_per_block"].as<int>();
	    }
	    LOG(INFO) << "Block Size: "
	              << page_per_block
	              << " pages";

	//load buffer_ram_size
	if (yaml_config["buffer_page_num"]) {
	    buffer_page_num = yaml_config["buffer_page_num"].as<int>();
	}
	LOG(INFO) << "Buffer Ram Size: "
			  << buffer_page_num
	          << " pages";

    if (yaml_config["ssd_path"]) {
        ssd_path = yaml_config["ssd_path"].as<std::string>();
    }
    LOG(INFO) << "ssd_path: "
              << ssd_path;

    if (yaml_config["dfs_path"]) {
        dfs_path = yaml_config["dfs_path"].as<std::string>();
    }
    LOG(INFO) << "dfs_path: "
              << dfs_path;

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
	return MOG_SUCCESS;
}

int32_t mog::allocate_memory() {
    for (auto device_id : devices) {
        gpu_caches[device_id] = new GpuCache(device_id);
        CHECK_EQ(gpu_caches[device_id]->initial(page_size, gpu_page_num), GPUCACHE_SUCCESS);
        CHECK_EQ(gpu_caches[device_id]->allocate_memory(), GPUCACHE_SUCCESS);
        LOG(INFO) << "---------> Create GPU Cache Done On Device " << device_id;
    }

    CHECK_EQ(CpuCache::singleton().initial(page_size, cpu_page_num), CPUCACHE_SUCCESS);
    CHECK_EQ(CpuCache::singleton().allocate_memory(), CPUCACHE_SUCCESS);
    LOG(INFO) << "---------> Create CPU Cache Done";
    CHECK_EQ(WriteBuffer::singleton().initial(page_size, page_per_block, buffer_page_num), WRITEBUFFER_SUCCESS);
    WriteBuffer::singleton().ssd_path = this->ssd_path;
    WriteBuffer::singleton().dfs_path = this->dfs_path;
    CHECK_EQ(WriteBuffer::singleton().allocate_memory(), WRITEBUFFER_SUCCESS);
    LOG(INFO) << "---------> Create Write Buffer Done ";

    SSDCache::singleton().dfs_path = this->dfs_path;
    SSDCache::singleton().ssd_path = this->ssd_path;
    SSDCache::singleton().max_block_num = this->SSD_page_num/this->page_per_block;
    SSDCache::singleton().initial();
    LOG(INFO) << "---------> Create SSD Cache Done ";

    return MOG_SUCCESS;
}

//! Create a new DB
int32_t mog::create() {

    boost::filesystem::path db_file_path_ssd = boost::filesystem::path(ssd_path + "/" +
                                                                       db_name+
                                                                       ".mog");

    boost::filesystem::path path_ssd = boost::filesystem::path(ssd_path);
    boost::filesystem::path path_dfs = boost::filesystem::path(dfs_path);

    boost::filesystem::remove_all(path_ssd);
    boost::filesystem::create_directories(db_file_path_ssd);

    boost::filesystem::remove_all(path_dfs);
    boost::filesystem::create_directories(path_dfs);


    CentraIndex::singleton().setup(ssd_path + "/" +
            db_name+
            ".mog");
    int32_t initial_block_id = 0;
    CentraIndex::singleton().put("FILE_NUM",
            strlen("FILE_NUM"),
            (char*)&initial_block_id,
            sizeof(initial_block_id));
    allocate_memory();
    LOG(INFO) << "Create a new Mog: "
              << this->db_name;
    return MOG_SUCCESS;
}

//! Load an existed DB
int32_t mog::load() {
    boost::filesystem::path db_file_path_ssd = boost::filesystem::path(ssd_path + "/");

    boost::filesystem::path path_ssd = boost::filesystem::path(ssd_path);
    boost::filesystem::remove_all(path_ssd);
    boost::filesystem::create_directory(path_ssd);

    std::string db_file_path_ssd_c = ssd_path + "/";
    std::string db_file_path_dfs_c = dfs_path + "/" + db_name+ ".mog";

    char cmd[256];
    sprintf(cmd, "cp -r %s %s", db_file_path_dfs_c.c_str(), db_file_path_ssd_c.c_str());
    system(cmd);

    CentraIndex::singleton().load(ssd_path + "/" +
                                    db_name+
                                    ".mog");

    LOG(INFO) << "Load an existed Mog: "
              << this->db_name;

    allocate_memory();

    return MOG_SUCCESS;
}

//! Open a DB, if it exists, load it; otherwise, create a DB with the name
/*!
    \param db_name the name of the db to open
    \return 0 if success
*/

int32_t mog::open(const std::string& db_name) {
    this->db_name = db_name;
    if (exist(db_name)) {
        load();
    } else {
        create();
    }
    return MOG_SUCCESS;
}

//! check the existence of a DB
/*!
    \param db_name the name of the db to check
    \return true if exists; otherwise false
 */
bool mog::exist(const std::string& db_name) {
    boost::filesystem::path db_file_path = boost::filesystem::path(dfs_path + "/" +
                                                                db_name+
                                                                ".mog");
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

int32_t mog::sync() {
    WriteBuffer::singleton().sync();
    SSDCache::singleton().sync();

    return MOG_SUCCESS;
}

int32_t mog::close() {
    sync();

    LOG(INFO) << "Sync Data to DFS\n";

    CentraIndex::singleton().close();

    boost::filesystem::path db_file_path_dfs = boost::filesystem::path(dfs_path + "/" +
                                                                    db_name+
                                                                    ".mog");

    std::string db_file_path_ssd_c = ssd_path + "/" + db_name+ ".mog";
    std::string db_file_path_dfs_c = dfs_path + "/";

    if (boost::filesystem::exists(db_file_path_dfs)) {
        boost::filesystem::remove_all(db_file_path_dfs);
    }

    char cmd[256];
    sprintf(cmd, "cp -r %s %s", db_file_path_ssd_c.c_str(), db_file_path_dfs_c.c_str());
    system(cmd);
    LOG(INFO) << "Update Index in DFS\n";

    boost::filesystem::path ssd_cache_path = boost::filesystem::path(ssd_path);
    boost::filesystem::remove_all(ssd_cache_path);
    LOG(INFO) << "Clear SSD Data\n";

    for (auto device_id : devices) {
        gpu_caches[device_id]->close();
    }
    LOG(INFO) << "Clean GPU Cache Data\n";

    CpuCache::singleton().close();
    LOG(INFO) << "Clean CPU Cache Data\n";

    WriteBuffer::singleton().close();
    LOG(INFO) << "Clean WriteBuffer Data\n";

    SSDCache::singleton().close();
    LOG(INFO) << "Clean SSD Cache Data\n";

    return MOG_SUCCESS;
}

int32_t mog::write(const std::string& key, const char *value, int32_t length) {
    WriteBuffer::singleton().write(key, value, length);
    return MOG_SUCCESS;
}


int32_t mog::gpu_read(const int32_t device_id, const std::string& key, char *value) {
    int32_t length = 0;
    length = this->gpu_caches[device_id]->read(key, value);
    return length;
}

int32_t mog::read(const std::string& key, char *value) {
    int32_t length = 0;

    length = WriteBuffer::singleton().read(key,value);
    if (length > 0) {
        return length;
    } else {
        length = CpuCache::singleton().read(key, value);
        return length;
    }
}

}
