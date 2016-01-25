//============================================================================
// Name        : CudaDB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "main.h"
#include "mog.h"

int main(int argc, char**argv) {

//	google::InitGoogleLogging(argv[0]);
//	google::LogToStderr();
//	google::SetLogDestination(google::INFO, logfile.c_str());
//	google::SetLogDestination(google::WARNING, logfile.c_str());
//	google::SetLogDestination(google::ERROR, logfile.c_str());
//	google::SetLogDestination(google::FATAL, logfile.c_str());

    cap::mog DB;
    CHECK_EQ(DB.open("www"), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre", "2", 1), MOG_SUCCESS);
    char bb[1024*1024*2];
    bb[0]='2';
    CHECK_EQ(DB.write("wgrfgre2", bb, 1024*1000), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre3", bb, 1024*1000), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre4", bb, 1024*1000), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre5", bb, 1024*1000), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre6", bb, 1024*1000), MOG_SUCCESS);
    CHECK_EQ(DB.write("wgrfgre7", bb, 1024*1000), MOG_SUCCESS);

   // CHECK_EQ(DB.write("wgrfgre2", bb, 1024*1000), MOG_SUCCESS);


    //CHECK_EQ(DB.write("wgrfgre2", bb, 1), MOG_SUCCESS);

    sleep(2);

    char aa[1024*1024*2];
    int32_t l = 0;
    l = DB.read("wgrfgre2", aa);
    l = DB.read("wgrfgre3", aa);
    l = DB.read("wgrfgre4", aa);
    l = DB.read("wgrfgre5", aa);
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

    while(1);
    return 0;
}
