cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(p1_forensic_probe C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_compile_options(
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

set(FIRMWARE_SOURCES
    src/main.c
    src/platform/platform_init.c
    src/transport/usb_transport.c
    src/transport/usb_descriptors.c
)

add_executable(dolos ${FIRMWARE_SOURCES})

target_include_directories(dolos PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/transport
)

target_link_libraries(dolos
    pico_stdlib
    hardware_gpio
    hardware_watchdog
    pico_multicore
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_uart(dolos 1)
pico_enable_stdio_usb(dolos 0)

pico_add_extra_outputs(dolos)