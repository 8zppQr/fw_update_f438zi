################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Soft/tinycrypt/source/aes_decrypt.c \
../Soft/tinycrypt/source/aes_encrypt.c \
../Soft/tinycrypt/source/cbc_mode.c \
../Soft/tinycrypt/source/ccm_mode.c \
../Soft/tinycrypt/source/cmac_mode.c \
../Soft/tinycrypt/source/ctr_mode.c \
../Soft/tinycrypt/source/ctr_prng.c \
../Soft/tinycrypt/source/ecc.c \
../Soft/tinycrypt/source/ecc_dh.c \
../Soft/tinycrypt/source/ecc_dsa.c \
../Soft/tinycrypt/source/ecc_platform_specific.c \
../Soft/tinycrypt/source/hmac.c \
../Soft/tinycrypt/source/hmac_prng.c \
../Soft/tinycrypt/source/sha256.c \
../Soft/tinycrypt/source/utils.c 

OBJS += \
./Soft/tinycrypt/source/aes_decrypt.o \
./Soft/tinycrypt/source/aes_encrypt.o \
./Soft/tinycrypt/source/cbc_mode.o \
./Soft/tinycrypt/source/ccm_mode.o \
./Soft/tinycrypt/source/cmac_mode.o \
./Soft/tinycrypt/source/ctr_mode.o \
./Soft/tinycrypt/source/ctr_prng.o \
./Soft/tinycrypt/source/ecc.o \
./Soft/tinycrypt/source/ecc_dh.o \
./Soft/tinycrypt/source/ecc_dsa.o \
./Soft/tinycrypt/source/ecc_platform_specific.o \
./Soft/tinycrypt/source/hmac.o \
./Soft/tinycrypt/source/hmac_prng.o \
./Soft/tinycrypt/source/sha256.o \
./Soft/tinycrypt/source/utils.o 

C_DEPS += \
./Soft/tinycrypt/source/aes_decrypt.d \
./Soft/tinycrypt/source/aes_encrypt.d \
./Soft/tinycrypt/source/cbc_mode.d \
./Soft/tinycrypt/source/ccm_mode.d \
./Soft/tinycrypt/source/cmac_mode.d \
./Soft/tinycrypt/source/ctr_mode.d \
./Soft/tinycrypt/source/ctr_prng.d \
./Soft/tinycrypt/source/ecc.d \
./Soft/tinycrypt/source/ecc_dh.d \
./Soft/tinycrypt/source/ecc_dsa.d \
./Soft/tinycrypt/source/ecc_platform_specific.d \
./Soft/tinycrypt/source/hmac.d \
./Soft/tinycrypt/source/hmac_prng.d \
./Soft/tinycrypt/source/sha256.d \
./Soft/tinycrypt/source/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Soft/tinycrypt/source/%.o Soft/tinycrypt/source/%.su Soft/tinycrypt/source/%.cyclo: ../Soft/tinycrypt/source/%.c Soft/tinycrypt/source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F439xx -c -I../Core/Inc -I"C:/Users/iisec/Documents/fw_update_f438zi/bootloader/Soft/tinycrypt/include" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Soft-2f-tinycrypt-2f-source

clean-Soft-2f-tinycrypt-2f-source:
	-$(RM) ./Soft/tinycrypt/source/aes_decrypt.cyclo ./Soft/tinycrypt/source/aes_decrypt.d ./Soft/tinycrypt/source/aes_decrypt.o ./Soft/tinycrypt/source/aes_decrypt.su ./Soft/tinycrypt/source/aes_encrypt.cyclo ./Soft/tinycrypt/source/aes_encrypt.d ./Soft/tinycrypt/source/aes_encrypt.o ./Soft/tinycrypt/source/aes_encrypt.su ./Soft/tinycrypt/source/cbc_mode.cyclo ./Soft/tinycrypt/source/cbc_mode.d ./Soft/tinycrypt/source/cbc_mode.o ./Soft/tinycrypt/source/cbc_mode.su ./Soft/tinycrypt/source/ccm_mode.cyclo ./Soft/tinycrypt/source/ccm_mode.d ./Soft/tinycrypt/source/ccm_mode.o ./Soft/tinycrypt/source/ccm_mode.su ./Soft/tinycrypt/source/cmac_mode.cyclo ./Soft/tinycrypt/source/cmac_mode.d ./Soft/tinycrypt/source/cmac_mode.o ./Soft/tinycrypt/source/cmac_mode.su ./Soft/tinycrypt/source/ctr_mode.cyclo ./Soft/tinycrypt/source/ctr_mode.d ./Soft/tinycrypt/source/ctr_mode.o ./Soft/tinycrypt/source/ctr_mode.su ./Soft/tinycrypt/source/ctr_prng.cyclo ./Soft/tinycrypt/source/ctr_prng.d ./Soft/tinycrypt/source/ctr_prng.o ./Soft/tinycrypt/source/ctr_prng.su ./Soft/tinycrypt/source/ecc.cyclo ./Soft/tinycrypt/source/ecc.d ./Soft/tinycrypt/source/ecc.o ./Soft/tinycrypt/source/ecc.su ./Soft/tinycrypt/source/ecc_dh.cyclo ./Soft/tinycrypt/source/ecc_dh.d ./Soft/tinycrypt/source/ecc_dh.o ./Soft/tinycrypt/source/ecc_dh.su ./Soft/tinycrypt/source/ecc_dsa.cyclo ./Soft/tinycrypt/source/ecc_dsa.d ./Soft/tinycrypt/source/ecc_dsa.o ./Soft/tinycrypt/source/ecc_dsa.su ./Soft/tinycrypt/source/ecc_platform_specific.cyclo ./Soft/tinycrypt/source/ecc_platform_specific.d ./Soft/tinycrypt/source/ecc_platform_specific.o ./Soft/tinycrypt/source/ecc_platform_specific.su ./Soft/tinycrypt/source/hmac.cyclo ./Soft/tinycrypt/source/hmac.d ./Soft/tinycrypt/source/hmac.o ./Soft/tinycrypt/source/hmac.su ./Soft/tinycrypt/source/hmac_prng.cyclo ./Soft/tinycrypt/source/hmac_prng.d ./Soft/tinycrypt/source/hmac_prng.o ./Soft/tinycrypt/source/hmac_prng.su ./Soft/tinycrypt/source/sha256.cyclo ./Soft/tinycrypt/source/sha256.d ./Soft/tinycrypt/source/sha256.o ./Soft/tinycrypt/source/sha256.su ./Soft/tinycrypt/source/utils.cyclo ./Soft/tinycrypt/source/utils.d ./Soft/tinycrypt/source/utils.o ./Soft/tinycrypt/source/utils.su

.PHONY: clean-Soft-2f-tinycrypt-2f-source

