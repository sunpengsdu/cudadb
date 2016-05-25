echo "g++ -c ./src/*.cpp -pthread  -std=c++11 -O2"
g++ -c ./src/*.cpp -pthread  -std=c++11  -O2
echo "g++ -o mog_server ./*.o -pthread -lzmq  -lboost_system -lboost_filesystem -lboost_thread  -lglog -lleveldb  -lyaml-cpp"
g++ -o mog_server ./*.o -pthread -lzmq  -lboost_system -lboost_filesystem -lboost_thread  -lglog -lleveldb  -lyaml-cpp
rm ./*.o
