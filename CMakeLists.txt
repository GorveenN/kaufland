cmake_minimum_required(VERSION 3.12)
project(UART C CXX)
set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(/opt/arm/stm32/inc)
include_directories(/opt/arm/stm32/CMSIS/Include)
include_directories(/opt/arm/stm32/CMSIS/Device/ST/STM32F4xx/Include)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THIRD_PARTY src/third/startup_stm32.c src/third/delay.c src/third/gpio.c src/third/sbrk.c)
set(MODULES src/buttons.c src/logger.c src/str_queue.c src/timer.c src/bluetooth.c src/leds.c)
set(APPLICATION main.c)
set(DCMAKE_TOOLCHAIN_FILE toolchain-stm32f411.cmake)

add_executable(${PROJECT_NAME}.elf ${APPLICATION} ${MODULES} ${THIRD_PARTY})

add_custom_target(bin
        COMMAND ${CMAKE_OBJCOPY} ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin -Obinary && cp compile_commands.json ../compile_commands.json
        DEPENDS ${PROJECT_NAME}.elf
        COMMENT "Preparing bin file: ${CMAKE_OBJCOPY} ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin -Obinary")

add_custom_target(send
        COMMAND sudo /opt/arm/stm32/ocd/qfn4
        DEPENDS bin
        COMMENT "SENDING")
