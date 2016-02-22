//============================================================================
// Name        : CudaDB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "main.h"
#include "mog.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char**argv) {

//	google::InitGoogleLogging(argv[0]);
//	google::LogToStderr();
//	google::SetLogDestination(google::INFO, logfile.c_str());
//	google::SetLogDestination(google::WARNING, logfile.c_str());
//	google::SetLogDestination(google::ERROR, logfile.c_str());
//	google::SetLogDestination(google::FATAL, logfile.c_str());

    cap::mog DB;
    CHECK_EQ(DB.open("www"), MOG_SUCCESS);
    DB.write("wgrfgre", "2", 1);
    char bb[1024*1024*2];
    bb[0]='2';
    char aa[1024*1024*2];
    int64_t l = 0;

    int64_t file_num = 0;

//    std::srand(std::time(0));
//    for (file_num=0; file_num <= 1000000; ++file_num) {
//        int64_t file_size = std::rand() % 1048576;
//        l = DB.write(std::to_string(file_num).c_str(), aa, file_size);
//    }

//    for (; file_num <= 100000; ++file_num) {
//        int64_t ttt = DB.insert_file(std::to_string(file_num).c_str(), "/home/sunp/18/race_data/asian_female/sy/__machine5_2015-11-21_0-0-0-1-94-24-42_2015-11-21_10-16-47_jpg_498_943_173_173.jpg");
//    }


   // LOG(INFO) << "##$#$#$$$$$$$$$$$$$$$$" << ttt;

    DB.write("wgrfgre2", bb, 1024*1000);
    DB.write("wgrfgre3", bb, 1024*1000);
    DB.write("wgrfgre4", bb, 1024*1000);
    DB.write("wgrfgre5", bb, 1024*1000);
    DB.write("wgrfgre6", bb, 1024*1000);
    DB.write("wgrfgre7", bb, 1024*1000);
    l = DB.read("wgrfgre5", aa);
   // CHECK_EQ(DB.write("wgrfgre2", bb, 1024*1000), MOG_SUCCESS);


    //CHECK_EQ(DB.write("wgrfgre2", bb, 1), MOG_SUCCESS);

    sleep(2);


    l = DB.read("wgrfgre2", aa);
    l = DB.read("wgrfgre3", aa);
    l = DB.read("wgrfgre4", aa);
    l = DB.read("wgrfgre5", aa);
    l = DB.read("wgrfgre", aa);
    l = DB.read("wgrfgre", aa);
    l = DB.read("wgrfgre6", aa);
    l = DB.read("wgrfgre7", aa);
    l = DB.read("wgrfgre4", aa);
    l = DB.read("wgrfgre5", aa);

    std::cout << l << "!!!" << aa << "@@@@@@@@@@@@@@\n";

    sleep(2);
    l = DB.read("wgrfgre2", aa);
    l = DB.read("wgrfgre3", aa);
    l = DB.read("wgrfgre4", aa);
    l = DB.read("wgrfgre5", aa);
    l = DB.read("wgrfgre6", aa);
    l = DB.read("wgrfgre7", aa);
    l = DB.read("wgrfgre4", aa);
    l = DB.read("wgrfgre5", aa);
    std::cout << l << "!!!" << aa << "@@@@@@@@@@@@@@\n";
    char* g_aa = cap::mog::malloc_gpu(0, 1024*1024*2);

    int64_t read_num = 0;
    std::srand(std::time(0));

    
    for (read_num=0; read_num <= 1000000; ++read_num) {
        int64_t read_id = std::rand() % 100000;
        l = DB.gpu_read(0, std::to_string(read_id).c_str(), g_aa);
    }

//    char* gpu_temp_p = mog_malloc_gpu(1, 1024*1024*2);
//    l = DB.gpu_read(1, "wgrfgre5", gpu_temp_p);
//    l = DB.gpu_read(1, "wgrfgre5", gpu_temp_p);
//    std::cout << l << "!!!" << aa << "@@@@@@@@@@@@@@\n";

    DB.close();

    return 0;
}
