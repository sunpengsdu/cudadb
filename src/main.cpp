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
    DB.open("www");
    while(1);
    return 0;
}
