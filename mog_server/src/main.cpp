//============================================================================
// Name        : CudaDB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include "main.h"
#include <pthread.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <cassert>
#include <zmq.hpp>

//std::shared_ptr<cap::mog> DB;
std::mutex mtx;
bool first = true;
bool reset_flag = false;
cap::mog *DB;

void CreateMogEntry() {
  reset_flag = true;
  if (DB != NULL) {
    DB->close();
  }
  if (first) {
    first = false;
    DB = new cap::mog();
  }
  DB->ready  = false;
  CHECK_EQ(DB->open("SensetimeMog"), MOG_SUCCESS);
  reset_flag = false;
  DB->ready = true;
  mtx.unlock();
}

void *worker_routine (void *arg) {
  zmq::context_t *context = (zmq::context_t *) arg;
  zmq::socket_t socket (*context, ZMQ_REP);
  socket.connect ("inproc://workers");
  char* mog_data = (char*)malloc(1024*1024*64);
  int64_t read_len = 0;
  while (true) {
    zmq::message_t request;
    socket.recv (&request);
    if (((char*)request.data())[0] == '!') {
      LOG(INFO) << "Receive Command to Clean Mog Data";
      DB->ready=false;
      if(mtx.try_lock()) {
        LOG(INFO) << "Clean Mog Data";
        std::thread create_mog(CreateMogEntry);
        create_mog.detach();
      } else { 
        sleep(1);
      }
      zmq::message_t tmp_reply(2);
      memcpy((void*)tmp_reply.data(), "OK", 2);
      socket.send(tmp_reply);
      continue;
    }
    // LOG(INFO) << "New Request: " << (char *)request.data() << " " << request.size();
    if (reset_flag) {
      int64_t len = 0;
      std::ifstream ifs((char *)request.data());
      if (! ifs.is_open()) {
        len = 0;
      }
      ifs.seekg(0, ifs.end);
      len = ifs.tellg();
      ifs.seekg(0, ifs.beg);
      if (len > 64*1024*1024) {
        ifs.close();
        len = 0;
      }
      char *buffer = new char[len+1];
      ifs.read(buffer, len);
      ifs.close();
      zmq::message_t reply(len);
      memcpy((void*)reply.data(), buffer, len);
      socket.send(reply);
      delete[] buffer;
    } else {
      read_len = DB->read_file((char *)request.data(), mog_data);
      zmq::message_t reply(read_len);
      memcpy((void*)reply.data(), mog_data, read_len);
      socket.send(reply);
    }
  }
  return (NULL);
}

int main(int argc, char**argv) {
  mtx.lock();
  std::thread create_mog(CreateMogEntry);
  zmq::context_t context(1);
  zmq::socket_t clients (context, ZMQ_ROUTER);
  boost::filesystem::path path_ipc = boost::filesystem::path("ipc:///tmp/SensetimeMog");
  boost::filesystem::remove_all(path_ipc);
  clients.bind ("ipc:///tmp/SensetimeMog");
  zmq::socket_t workers (context, ZMQ_DEALER);
  workers.bind ("inproc://workers");
  for (int thread_nbr = 0; thread_nbr != 10; thread_nbr++) {
    pthread_t worker;
    pthread_create (&worker, NULL, worker_routine, (void *) &context);
  }
  zmq_proxy(static_cast<void *>(clients), static_cast<void *>(workers), NULL);
  return 0;
}
