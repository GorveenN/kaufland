set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)
set(CMAKE_CROSSCOMPILING 1)


set(CMAKE_C_COMPILER arm-eabi-gcc)
set(CMAKE_OBJCOPY arm-eabi-objcopy)
set(CMAKE_OBJDUMP arm-eabi-objdump)

set(FLAGS "-DSTM32F411xE \
    -mthumb -mcpu=cortex-m4 \
    -O2 -g \
    -Wall -Wextra \
    -Wno-unused-parameter \
    -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS ${FLAGS})

set(CMAKE_EXE_LINKER_FLAGS  "-mthumb -mcpu=cortex-m4 \
    -Wl,--gc-sections -nostartfiles -L/opt/arm/stm32/lds -Tstm32f411re.lds")
