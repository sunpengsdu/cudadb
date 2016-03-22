/*
 * mog.h
 *
 *  Created on: 1 Mar 2016
 *      Author: sunp
 */

#ifndef MOG_H_
#define MOG_H_

#include <iostream>
#include <string>
#include <zmq.hpp>

namespace st {

class mog {

public:
    mog();
    mog(std::string ipc_addr);
    size_t get(const std::string file_name, std::string &data);
    int32_t exists();

private:
    zmq::context_t *context;
    zmq::socket_t  *socket;
    int32_t connected;
};

mog::mog() {
	this->connected = -1;
    this->context = new zmq::context_t(1);
    this->socket  = new zmq::socket_t(*context, ZMQ_REQ);
    this->socket->connect ("ipc:///tmp/SensetimeMog");
}

mog::mog(std::string ipc_addr) {
	this->connected = -1;
    this->context = new zmq::context_t(1);
    this->socket  = new zmq::socket_t(*context, ZMQ_REQ);
    this->socket->connect (ipc_addr.c_str());
}

int32_t mog::exists() {
	return this->connected;
}

size_t mog::get(const std::string file_name, std::string &data) {
	zmq::message_t request (file_name.length()+1);
    zmq::message_t reply;
    memcpy ((char*)request.data(), file_name.c_str(), file_name.length());
    this->socket->send (request);

    this->socket->recv (&reply);
    data.clear();
    data.append((char*)reply.data(), reply.size());
    return reply.size();
}

}

#endif /* MOG_H_ */
