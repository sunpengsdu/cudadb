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
    for (int i = 0; i < 4500; ++i) {
        CHECK_EQ(DB.write("wgrfgre", "2", 1), MOG_SUCCESS);
    }
    while(1);
    return 0;
}
