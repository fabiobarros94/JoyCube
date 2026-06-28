cmake_minimum_required(VERSION 3.13)

# Initialize Pico SDK if not already initialized
include(pico_sdk_import.cmake OPTIONAL)

project(JoyCube C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Define the executable
add_executable(joycube
    main.c
    usb_descriptors.c
)

# Link dependencies (Pico stdlib, TinyUSB, and ADC)
target_link_libraries(joycube
    pico_stdlib
    hardware_adc
    tinyusb_device
    tinyusb_board
)

# Enable USB support and disable standard UART stdio (useful for USB debugging)
pico_enable_stdio_usb(joycube 1)
pico_enable_stdio_uart(joycube 0)

# Generate binary files (uf2, hex, bin)
pico_add_extra_outputs(joycube)
