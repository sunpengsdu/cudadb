g++ -c ./*.cpp -std=c++11
nvcc -c ./*.cu
nvcc -o main ./*.o -lboost_system -lboost_filesystem -lboost_thread  -lglog -lleveldb  -lyaml-cpp -lcuda
rm ./*.o
