################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/extApi.c \
../src/extApiPlatform.c 

CPP_SRCS += \
../src/eBot_Sandbox.cpp \
../src/eBot_Sim_Predef.cpp \
../src/search_n_rescue.cpp 

OBJS += \
./src/eBot_Sandbox.o \
./src/eBot_Sim_Predef.o \
./src/extApi.o \
./src/extApiPlatform.o \
./src/search_n_rescue.o 

C_DEPS += \
./src/extApi.d \
./src/extApiPlatform.d 

CPP_DEPS += \
./src/eBot_Sandbox.d \
./src/eBot_Sim_Predef.d \
./src/search_n_rescue.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -DDO_NOT_USE_SHARED_MEMORY -I"/home/aditya/Desktop/CS684_Project/Task 1A Code/search_n_rescue/remoteApi" -I"/home/aditya/Desktop/CS684_Project/Task 1A Code/search_n_rescue/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -DNON_MATLAB_PARSING -DDO_NOT_USE_SHARED_MEMORY -DMAX_EXT_API_CONNECTIONS=255 -I"/home/aditya/Desktop/CS684_Project/Task 1A Code/search_n_rescue/include" -I"/home/aditya/Desktop/CS684_Project/Task 1A Code/search_n_rescue/remoteApi" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


