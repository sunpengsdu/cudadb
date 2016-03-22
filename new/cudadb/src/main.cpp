//============================================================================
// Name        : CudaDB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "main.h"

int main(int argc, char**argv) {

//	google::InitGoogleLogging(argv[0]);
//	google::LogToStderr();
//	google::SetLogDestination(google::INFO, logfile.c_str());
//	google::SetLogDestination(google::WARNING, logfile.c_str());
//	google::SetLogDestination(google::ERROR, logfile.c_str());
//	google::SetLogDestination(google::FATAL, logfile.c_str());

    cap::mog *DB;
    DB = new cap::mog();
    CHECK_EQ(DB->open("SensetimeMog"), MOG_SUCCESS);

    zmq::context_t context(1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("ipc:///tmp/SensetimeMog");
    char* mog_data = (char*)malloc(1024*1024*1.5);

    int64_t read_len = 0;

    while (true) {
        zmq::message_t request;
        socket.recv (&request);
        LOG(INFO) << "New Request: " << (char *)request.data() << " " << request.size();
        read_len = DB->read_file((char *)request.data(), mog_data);

        zmq::message_t reply(read_len);
        memcpy((void*)reply.data(), mog_data, read_len);
        socket.send(reply);
    }
//
//    int64_t l = 0;
//    int64_t file_num = 0;
//
//    for (; file_num <= 100000; ++file_num) {
//        int64_t ttt = DB->insert_file(std::to_string(file_num).c_str(), "/home/sunp/18/race_data/asian_female/sy/__machine5_2015-11-21_0-0-0-1-94-24-42_2015-11-21_10-16-47_jpg_498_943_173_173.jpg");
//    }
//
//    sleep(2);
//
//    char* g_aa = (char*)malloc(1024*1024*2);
//
//    int64_t read_num = 0;
//    std::srand(std::time(0));
//
//    for (read_num=0; read_num <= 1000000; ++read_num) {
//        int64_t read_id = std::rand() % 100000;
//        l = DB->read(std::to_string(read_id).c_str(), g_aa);
//    }
    DB->close();

    return 0;
}
