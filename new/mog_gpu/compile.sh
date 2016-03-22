g++ -c ./*.cpp  -std=c++11
nvcc -c ./*.cu
nvcc -o main ./*.o -lboost_system -lboost_thread  -lglog -lcuda
rm ./*.o
