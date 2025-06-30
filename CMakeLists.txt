# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

# Disable Google Test
set(BUILD_GMOCK OFF)
set(BUILD_GTEST OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (DEFINED ENV{DEFY_FW_VERSION})
    set(ENV_FW_VERSION $ENV{DEFY_FW_VERSION})
    message("Using DEFY_FW_VERSION from environment ('${ENV_FW_VERSION}')")
endif ()

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

# Include directories for HAL and middleware libraries
include_directories(src)
include_directories(configuration/middleware_config)

# Libraries
include_directories(lib/KeyboardioHID/src)
include_directories(lib/NeuronLedLibrary/)
include_directories(lib/NeuronLedLibrary/LedModeSerializable/)
include_directories(lib/NeuronLedLibrary/LedModeSerializable/src/)
include_directories(lib/RP_platform)
include_directories(lib/RP_platform/hal)
include_directories(lib/RP_platform/hal/mcu)
include_directories(lib/RP_platform/hal/mcu/rp20)
include_directories(lib/RP_platform/middleware)
include_directories(lib/RP_platform/middleware/halsep)

# Tell CMake where to find the executable source file
set(NEURONWIRED NeuronWired)

add_executable(${NEURONWIRED}
        src/DefyFirmwareVersion.cpp
        src/EEPROMPadding.cpp
        src/EEPROMUpgrade.cpp
        src/hid_report_descriptor.cpp
        src/IntegrationTest.cpp
        src/LED-CapsLockLight.cpp
        src/main.cpp
        src/config_app.h

        lib/KeyboardioHID/src/MultiReport/ConsumerControl.cpp
        lib/KeyboardioHID/src/MultiReport/Keyboard.cpp
        lib/KeyboardioHID/src/MultiReport/Mouse.cpp
        lib/KeyboardioHID/src/MultiReport/SystemControl.cpp
        lib/KeyboardioHID/src/hidDefy.cpp
        lib/KeyboardioHID/src/HIDReportObserver.cpp
        
        # Colormap includes
        lib/NeuronLedLibrary/Colormap-Defy.cpp
        lib/NeuronLedLibrary/LEDEffect-BatteryStatus.cpp
        lib/NeuronLedLibrary/LEDEffect-Bluetooth-Pairing-Defy.cpp
        lib/NeuronLedLibrary/LEDEffect-Rainbow-Defy.cpp
        lib/NeuronLedLibrary/LEDEffect-SolidColor-Defy.cpp
        lib/NeuronLedLibrary/LEDEffect-Stalker-Defy.cpp
        lib/NeuronLedLibrary/LED-Palette-Theme-Defy.cpp

        lib/SPISlave/src/link/spi_link_def.h
        lib/SPISlave/src/link/spi_link_slave.h
        lib/SPISlave/src/link/spi_link_slave.c

        lib/RP_platform/middleware/memory/heap.c
        lib/RP_platform/middleware/utils/dl_crc32.c
        lib/RP_platform/middleware/halsep/hal_mcu_spi.c
        lib/RP_platform/middleware/halsep/hal_mcu_dma.h
        lib/RP_platform/middleware/halsep/hal_mcu_dma.c

        # Low level hal includes
        lib/RP_platform/hal/mcu/rp20/hal_ll_rp20xx_gpio.c
        lib/RP_platform/hal/mcu/rp20/hal_ll_rp20xx_mutex.c
        lib/RP_platform/hal/mcu/rp20/hal_ll_rp20xx_spi.c
        lib/RP_platform/hal/mcu/hal_mcu_spi_ll.h
        lib/RP_platform/hal/mcu/hal_mcu_dma_ll.h
        lib/RP_platform/hal/mcu/rp20/hal_ll_rp20xx_dma.c

        lib/RP_platform/middleware/dl_types.h
        lib/RP_platform/middleware/halsep/hal_mcu_mutex.c
        src/Watchdog_timer.h
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
        PluginUpgrade
        SPISlave
        Time_counter
        FIFO_BUFFER
        )
# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${NEURONWIRED})

target_compile_definitions(${NEURONWIRED} PUBLIC
        -DF_CPU=133000000L
        -D__RP2040__
        -DUSE_TINYUSB
        -DBOARD_VENDORID=0x35EF
        -DBOARD_PRODUCTID=0x0010
        -DBOARD_MANUFACTURER="DYGMA"
        -DBOARD_PRODUCT="DEFY"
        -DUSBCON
        -DKALEIDOSCOPE_HARDWARE_H="Kaleidoscope-Hardware-Dygma-DefyWN.h"
        -DKALEIDOSCOPE_LED_CONTROL_H="kaleidoscope/plugin/LEDControl.h"
        -DEEPROM_EMULATION_SIZE=8192
        -DHAL_CFG_MCU=HAL_MCU_RP2040
        -DMCU_ALIGNMENT_SIZE=4
        -DNEURON_WIRED
        -DUSB_PRODUCT="DEFY"

		-DARDUINO_RASPBERRY_PI_PICO
        -DARDUINO_ARCH_RP2040
        -DARDUINO=10804
        -DCFG_DEBUG=0
        -DLFS_NO_ASSERT
        -DUSE_LFXO
        )

# Defy Neuron version
target_compile_definitions(${NEURONWIRED} PUBLIC
        -DDEFY_NEURON_FW_VERSION=\"v2.1.0\"
       )


# Defy firmware package version - Neuron + Keyscanner + RFGW
if (DEFINED ENV_FW_VERSION)
target_compile_definitions(${NEURONWIRED} PUBLIC
        -DDEFY_FW_VERSION=\"${ENV_FW_VERSION}\"
       )
else ()
target_compile_definitions(${NEURONWIRED} PUBLIC
        -DDEFY_FW_VERSION=\"v2.1.1-alpha\"          # Fixed "alpha" value to recognize local builds
       )
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${NEURONWIRED} PRIVATE -O0)
    target_compile_definitions(${NEURONWIRED} PUBLIC
        DEBUG)
else (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(${NEURONWIRED} PRIVATE -O3)
endif ()

configure_file(memmap_mainProgram.ld memmap_mainProgram.ld @ONLY)
pico_set_linker_script(${NEURONWIRED} ${CMAKE_CURRENT_BINARY_DIR}/memmap_mainProgram.ld)


add_custom_target(${PROJECT} ALL DEPENDS ${NEURONWIRED})
