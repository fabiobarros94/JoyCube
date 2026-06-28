cmake_minimum_required(VERSION 3.13)

# Inicializa o Pico SDK se não estiver inicializado
include(pico_sdk_import.cmake OPTIONAL)

project(JoyCube C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK do Raspberry Pi Pico
pico_sdk_init()

# Define o executável
add_executable(joycube
    main.c
    usb_descriptors.c
)

# Linka as dependências (Pico stdlib, TinyUSB e ADC)
target_link_libraries(joycube
    pico_stdlib
    hardware_adc
    tinyusb_device
    tinyusb_board
)

# Habilita suporte a USB e desabilita UART para stdio (opcional, mas bom para debug via USB)
pico_enable_stdio_usb(joycube 1)
pico_enable_stdio_uart(joycube 0)

# Cria os arquivos de binário (uf2, hex, bin)
pico_add_extra_outputs(joycube)
