g++ -c ./*.cpp -pthread  -std=c++11
g++ -o main ./*.o -pthread -lzmq  -lboost_system -lboost_filesystem -lboost_thread  -lglog -lleveldb  -lyaml-cpp
rm ./*.o
