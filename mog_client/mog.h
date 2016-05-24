/*
 * mog.h
 *
 *  Created on: 1 Mar 2016
 *      Author: sunp
 */

#ifndef MOG_H_
#define MOG_H_

#define SOCKETNUM 5

#include <iostream>
#include <string>
#include <zmq.hpp>
#include <mutex>
#include <vector>
#include <atomic>

namespace st {
class mog {
public:
    mog();
    mog(std::string ipc_addr);
    size_t get(const std::string file_name, std::string &data);
    int32_t exists();
	static mog& GetInstance();
private:
    zmq::context_t *context;
    //zmq::socket_t  *socket;
    int32_t connected;
	zmq::socket_t*   socket_pool[SOCKETNUM];
	std::mutex       lock_pool[SOCKETNUM];
	std::atomic_flag a_lock_pool[SOCKETNUM]={ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT};
};

mog::mog() {
	this->connected = -1;
    this->context = new zmq::context_t(1);
	for (int32_t i=0; i<SOCKETNUM; ++i) {
		socket_pool[i] = new zmq::socket_t(*context, ZMQ_REQ);
		socket_pool[i]->connect ("ipc:///dev/shm/SensetimeMog");
	}
}

mog::mog(std::string ipc_addr) {
	//this->count = false;
	this->connected = -1;
    this->context = new zmq::context_t(1);
	for (int32_t i=0; i<SOCKETNUM; ++i) {
		socket_pool[i] = new zmq::socket_t(*context, ZMQ_REQ);
		//socket_pool[i]->connect ("ipc:///tmp/SensetimeMog");
		socket_pool[i]->connect ("tcp://127.0.0.1:9555");
	}
}

mog& mog::GetInstance() {
	static mog instance;
	return instance;
}

int32_t mog::exists() {
	return this->connected;
}  

size_t mog::get(const std::string file_name, std::string &data) {

	zmq::message_t request (file_name.length()+1);
    zmq::message_t reply;
    memset((char*)request.data(), 0, file_name.length()+1);
    memcpy ((char*)request.data(), file_name.c_str(), file_name.length());
	while(true) {
		for (int32_t i=0; i<SOCKETNUM; ++i) {
			//if (lock_pool[i].try_lock()) {
			if (!a_lock_pool[i].test_and_set()) {
				socket_pool[i]->send (request);
			    socket_pool[i]->recv (&reply);
				data.clear();
				data.append((char*)reply.data(), reply.size());
				//lock_pool[i].unlock();
				a_lock_pool[i].clear();
				return reply.size();
			}
		}
		std::cout << "#######\n";
	}
}

}

#endif /* MOG_H_ */
