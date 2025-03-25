################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../biblioteca.c \
../libro.c \
../main.c \
../menu.c \
../usuario.c 

C_DEPS += \
./biblioteca.d \
./libro.d \
./main.d \
./menu.d \
./usuario.d 

OBJS += \
./biblioteca.o \
./libro.o \
./main.o \
./menu.o \
./usuario.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./biblioteca.d ./biblioteca.o ./libro.d ./libro.o ./main.d ./main.o ./menu.d ./menu.o ./usuario.d ./usuario.o

.PHONY: clean--2e-

