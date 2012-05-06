################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CommandLineInterface.cpp \
../src/EdgeDetection.cpp \
../src/FYP.cpp \
../src/FYPMain.cpp \
../src/boosttest.cpp \
../src/kmeans_test.cpp 

OBJS += \
./src/CommandLineInterface.o \
./src/EdgeDetection.o \
./src/FYP.o \
./src/FYPMain.o \
./src/boosttest.o \
./src/kmeans_test.o 

CPP_DEPS += \
./src/CommandLineInterface.d \
./src/EdgeDetection.d \
./src/FYP.d \
./src/FYPMain.d \
./src/boosttest.d \
./src/kmeans_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


