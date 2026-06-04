cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(p1_forensic_probe VERSION 0.1.0 LANGUAGES C CXX ASM)
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
    src/target/target_ctrl.c        
    src/transport/usb_transport.c
    src/transport/usb_descriptors.c
    src/swd/swd_proto.c
    src/swd/swd_phy.S
    src/app/app.c
    src/display/lcd.c
)

add_executable(dolos ${FIRMWARE_SOURCES})

target_include_directories(dolos PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display
)

target_link_libraries(dolos
    pico_stdlib
    hardware_gpio
    hardware_watchdog
    pico_multicore
    tinyusb_device
    tinyusb_board
    hardware_i2c
)

pico_enable_stdio_uart(dolos 1)
pico_enable_stdio_usb(dolos 1)

pico_add_extra_outputs(dolos)