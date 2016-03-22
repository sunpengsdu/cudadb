#include <zmq.hpp>
#include <string>
#include <iostream>

int main() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);
    socket.connect ("ipc:///tmp/SensetimeMog");

    std::string file_name = "/home/sunp/18/race_data/asian_female/sy/__machine5_2015-11-21_0-0-0-1-94-24-42_2015-11-21_10-16-47_jpg_498_943_173_173.jpg";

    for (int request_nbr = 0; request_nbr != 1; request_nbr++) {
        zmq::message_t request (1024);
        memcpy (request.data(),file_name.c_str(), file_name.length());
        std::cout << "Sending Request " << request_nbr << "…" << std::endl;
        socket.send (request);

        //  Get the reply.
        zmq::message_t reply;
        socket.recv (&reply);
        std::cout << "Received " << request_nbr << std::endl;
    }
    return 0;
}
