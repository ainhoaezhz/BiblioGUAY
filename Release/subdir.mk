################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bd.c \
../biblioteca.c \
../libro.c \
../prestamo.c \
../servidor.c \
../sqlite3.c \
../usuario.c 

C_DEPS += \
./bd.d \
./biblioteca.d \
./libro.d \
./prestamo.d \
./servidor.d \
./sqlite3.d \
./usuario.d 

OBJS += \
./bd.o \
./biblioteca.o \
./libro.o \
./prestamo.o \
./servidor.o \
./sqlite3.o \
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
	-$(RM) ./bd.d ./bd.o ./biblioteca.d ./biblioteca.o ./libro.d ./libro.o ./prestamo.d ./prestamo.o ./servidor.d ./servidor.o ./sqlite3.d ./sqlite3.o ./usuario.d ./usuario.o

.PHONY: clean--2e-

