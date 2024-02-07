# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include build functions from Pico SDK
include(pico_sdk_import.cmake)
#include(pico_extras_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(NeuronWired C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Add subdirectories to the project
add_subdirectory(lib)

# Tell CMake where to find the executable source file
set(NEURONWIRED NeuronWired)

add_executable(${NEURONWIRED}
        src/DefyFirmwareVersion.cpp
        src/EEPROMPadding.cpp
        src/EEPROMUpgrade.cpp
        src/IntegrationTest.cpp
        src/LED-CapsLockLight.cpp
        src/main.cpp
        )

pico_set_program_name(${NEURONWIRED} "NeuronWired")
pico_set_program_version(${NEURONWIRED} "0.1")

# Enable usb output, disable uart output
pico_enable_stdio_uart(${NEURONWIRED} 0)
pico_enable_stdio_usb(${NEURONWIRED} 0)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${NEURONWIRED}
        pico_stdlib
        pico_multicore
        pico_unique_id
        hardware_i2c
        hardware_exception
        hardware_dma
        hardware_flash
        hardware_pio
        # hardware_sleep
        hardware_spi
        )

# Add any user requested libraries
target_link_libraries(${NEURONWIRED}
        Arduino
        Communications
        CRC
        EEPROM
        kaleidoscope
        KeyboardioHID
        PluginLedLibrary
        PluginUpgrade
        SPISlave
        )
# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${NEURONWIRED})

# target_compile_options(${NEURONWIRED} PRIVATE -O0)

target_compile_definitions(${NEURONWIRED} PUBLIC
        -DNEURONWIRED=1
        -DFMW_VERSION=${VERSION}
        -DBOARD_VENDORID=0x35EF
        -DBOARD_PRODUCTID=0x0010
        -DBOARD_MANUFACTURER="DYGMA"
        -DBOARD_PRODUCT="DEFY"
        -DUSB_PRODUCT="DEFY"

        -DF_CPU=133000000L
        -DEEPROM_EMULATION_SIZE=8192
        -DKALEIDOSCOPE_HARDWARE_H="Kaleidoscope-Hardware-Dygma-DefyWN.h"
        -DKALEIDOSCOPE_LED_CONTROL_H="kaleidoscope/plugin/LEDControl.h"
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)

    math(EXPR FLASH_OFFSET_MAINPROGRAM "0")

    target_compile_definitions(${NEURONWIRED} PUBLIC
            DBG_PRINT_ENABLED
            DBG_PRINT_ERROR_ENABLED
            DBG_PRINT_TRACE_ENABLED)

else (CMAKE_BUILD_TYPE MATCHES Release)
    math(EXPR FLASH_OFFSET_MAINPROGRAM "0")
endif ()

configure_file(memmap_mainProgram.ld memmap_mainProgram.ld @ONLY)
pico_set_linker_script(${NEURONWIRED} ${CMAKE_CURRENT_BINARY_DIR}/memmap_mainProgram.ld)


add_custom_target(${PROJECT} ALL DEPENDS ${NEURONWIRED})
