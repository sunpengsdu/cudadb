################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/CudaDB.o \
../src/DB.o \
../src/DB_GPU.o 

CPP_SRCS += \
../src/CudaDB.cpp \
../src/DB.cpp 

OBJS += \
./src/CudaDB.o \
./src/DB.o 

CPP_DEPS += \
./src/CudaDB.d \
./src/DB.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/nvidia-346/cuda -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


