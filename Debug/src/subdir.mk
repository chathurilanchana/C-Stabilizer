################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Client.cpp \
../src/Data.cpp \
../src/Label.cpp \
../src/MultiplexerServer.cpp \
../src/ProcessingThread.cpp \
../src/ReceivedMessage.cpp \
../src/Server.cpp \
../src/ServerAPI.cpp \
../src/SocketManager.cpp \
../src/ThreadUtil.cpp \
../src/Timer.cpp 

OBJS += \
./src/Client.o \
./src/Data.o \
./src/Label.o \
./src/MultiplexerServer.o \
./src/ProcessingThread.o \
./src/ReceivedMessage.o \
./src/Server.o \
./src/ServerAPI.o \
./src/SocketManager.o \
./src/ThreadUtil.o \
./src/Timer.o 

CPP_DEPS += \
./src/Client.d \
./src/Data.d \
./src/Label.d \
./src/MultiplexerServer.d \
./src/ProcessingThread.d \
./src/ReceivedMessage.d \
./src/Server.d \
./src/ServerAPI.d \
./src/SocketManager.d \
./src/ThreadUtil.d \
./src/Timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


