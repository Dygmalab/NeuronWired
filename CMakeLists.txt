# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(DEVICE_ID 0x4b534352)
SET(BOOTLOADER_VERSION 1)
SET(SEAL_VERSION 1)
SET(HARDWARE_VERSION 1)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (NOT DEFINED DEBUG_BOOTLOADER)
        math(EXPR BOOTLOADER_SIZE "28")
    else ()
        math(EXPR BOOTLOADER_SIZE "40")
    endif ()
else (CMAKE_BUILD_TYPE MATCHES Release)
    if (NOT DEFINED DEBUG_BOOTLOADER)
        math(EXPR BOOTLOADER_SIZE "20")
    else ()
        math(EXPR BOOTLOADER_SIZE "32")
    endif ()
endif ()

if (NOT DEFINED VERSION)
    #Version defined as 0x00001000 means 0.1.0 or 0x01001001 meas 1.1.1 that is 0x00.000.000
    math(EXPR VERSION "0x01000000" OUTPUT_FORMAT HEXADECIMAL)
    message("VERSION NOT DEFINED SETTING=${VERSION}")
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

# pico_generate_pio_header(${NEURONWIRED} ${CMAKE_CURRENT_LIST_DIR}/headers/WS2812.pio)

#target_include_directories(${NEURONWIRED} PRIVATE
#        headers
#        )
        
#target_sources(${NEURONWIRED} INTERFACE
#
#        )

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
        
		# WS2812
        # SPI
        # LEDTypes
        # IS31FL3743B
        # Debug_print
        # EEPROM
        # LEDManagement
        # LEDEffects
        # KeyScanner
        # Communications
        # RFCommunications
        # I2C
        # BatteryManagement
        # PinDeclarations
        # ModuleCommunications
        )
# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${NEURONWIRED})

target_compile_options(${NEURONWIRED} PRIVATE -O0)

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
    ##Change this to 0 if you want to debug
    #pico_enable_stdio_usb(${NEURONWIRED} 1)

    math(EXPR FLASH_OFFSET_MAINPROGRAM "0")

    target_compile_definitions(${NEURONWIRED} PUBLIC
            DBG_PRINT_ENABLED
            DBG_PRINT_ERROR_ENABLED
            DBG_PRINT_TRACE_ENABLED)

else (CMAKE_BUILD_TYPE MATCHES Release)
    math(EXPR FLASH_OFFSET_MAINPROGRAM "${BOOTLOADER_SIZE} * 1024 + 4*1024")
endif ()

configure_file(memmap_mainProgram.ld memmap_mainProgram.ld @ONLY)
pico_set_linker_script(${NEURONWIRED} ${CMAKE_CURRENT_BINARY_DIR}/memmap_mainProgram.ld)


# #Bootloader
# ################################################################################

# set(BOOTLOADER NeuronWired_bootloader)

# add_executable(${BOOTLOADER}
        # bootloader.cpp
        # )

# target_include_directories(${BOOTLOADER} PRIVATE
        # headers
        # )
# # Link with the pico stdlib
# target_link_libraries(${BOOTLOADER} PRIVATE
        # cmsis_core
        # pico_stdlib
        # IS31FL3743B
        # hardware_sync
        # hardware_resets
        # hardware_structs
        # hardware_i2c
        # hardware_flash
        # hardware_dma
        # pico_multicore)

# pico_add_extra_outputs(${BOOTLOADER})
# target_compile_options(${BOOTLOADER} PRIVATE -Wall -Wextra -Os)
# target_compile_definitions(${BOOTLOADER}
        # PUBLIC
        # BOOTLOADER_SIZE=${BOOTLOADER_SIZE}
        # SEAL_VERSION=${SEAL_VERSION}
        # BOOTLOADER_VERSION=${BOOTLOADER_VERSION}
        # HARDWARE_VERSION=${HARDWARE_VERSION}
        # DEVICE_ID=${DEVICE_ID}
        # )


# if (DEFINED DEBUG_BOOTLOADER)
    # pico_enable_stdio_usb(${BOOTLOADER} 1)
    # target_compile_definitions(${BOOTLOADER} PUBLIC DEBUG_BOOTLOADER=1)
# else ()
    # pico_enable_stdio_usb(${BOOTLOADER} 0)
# endif ()
# # Enable USB serial
# pico_enable_stdio_uart(${BOOTLOADER} 0)

# math(EXPR FLASH_OFFSET_BOOTLOADER "0")
# math(EXPR FLASH_LENGTH_BOOTLOADER "${BOOTLOADER_SIZE} * 1024")
# configure_file(memmap_bootloader.ld memmap_bootloader.ld @ONLY)
# pico_set_linker_script(${BOOTLOADER} ${CMAKE_CURRENT_BINARY_DIR}/memmap_bootloader.ld)

# Combine the bootloader and application into one flashable UF2 image
################################################################################

# set(COMPLETE_UF2 ${CMAKE_CURRENT_BINARY_DIR}/NEURONWIRED_MAIN_FW)

# find_package(Python3 REQUIRED COMPONENTS Interpreter)
# add_custom_command(OUTPUT ${COMPLETE_UF2} DEPENDS ${BOOTLOADER} ${NEURONWIRED}
        # COMMENT "Building full UF2 image"
        # COMMAND ${Python3_EXECUTABLE}
        # ${CMAKE_CURRENT_SOURCE_DIR}/mergeBootloaderWithFirmware.py
        # -v ${VERSION}
        # -vs ${SEAL_VERSION}
        # -o ${COMPLETE_UF2}
        # ${BOOTLOADER} ${NEURONWIRED}
        # )

# add_custom_target(${PROJECT} ALL DEPENDS ${COMPLETE_UF2})

# install(FILES ${COMPLETE_UF2} DESTINATION ${CMAKE_INSTALL_PREFIX})

# set(COMPLETE_UF2 ${CMAKE_CURRENT_BINARY_DIR}/NEURONWIRED_MAIN_FW)

# find_package(Python3 REQUIRED COMPONENTS Interpreter)
# add_custom_command(OUTPUT ${COMPLETE_UF2} DEPENDS ${BOOTLOADER} ${NEURONWIRED}
        # COMMENT "Building full UF2 image"
        # COMMAND ${Python3_EXECUTABLE}
        # ${CMAKE_CURRENT_SOURCE_DIR}/mergeBootloaderWithFirmware.py
        # -v ${VERSION}
        # -vs ${SEAL_VERSION}
        # -o ${COMPLETE_UF2}
        # ${BOOTLOADER} ${NEURONWIRED}
        # )

add_custom_target(${PROJECT} ALL DEPENDS ${NEURONWIRED})

# install(FILES ${COMPLETE_UF2} DESTINATION ${CMAKE_INSTALL_PREFIX})



# #Tests jig
# ################################################################################

# #First phase
# #####################################################################################

# set(TEST_JIG_FIRST_PHASE test_jig_first_phase)


# # Tell CMake where to find the executable source file
# add_executable(${TEST_JIG_FIRST_PHASE}
        # testjig_first_phase.cpp
        # )

# pico_generate_pio_header(${TEST_JIG_FIRST_PHASE} ${CMAKE_CURRENT_LIST_DIR}/headers/WS2812.pio)

# pico_set_program_name(${TEST_JIG_FIRST_PHASE} "Test Jig First Phase")
# pico_set_program_version(${TEST_JIG_FIRST_PHASE} "0.1")

# # Enable usb output, disable uart output
# pico_enable_stdio_uart(${TEST_JIG_FIRST_PHASE} 0)
# pico_enable_stdio_usb(${TEST_JIG_FIRST_PHASE} 1)

# # Link to pico_stdlib (gpio, time, etc. functions)

# target_include_directories(${TEST_JIG_FIRST_PHASE} PRIVATE
        # headers
        # )

# # Link to pico_stdlib (gpio, time, etc. functions)
# target_link_libraries(${TEST_JIG_FIRST_PHASE}
        # pico_stdlib
        # pico_multicore
        # hardware_exception
        # hardware_dma
        # hardware_adc
        # hardware_flash
        # hardware_sleep
        # )

# # Add any user requested libraries
# target_link_libraries(${TEST_JIG_FIRST_PHASE}
        # WS2812
        # SPI
        # LEDTypes
        # IS31FL3743B
        # Debug_print
        # EEPROM
        # LEDManagement
        # LEDEffects
        # KeyScanner
        # Communications
        # RFCommunications
        # I2C
        # PinDeclarations
        # ModuleCommunications
        # BatteryManagement
        # )

# # Create map/bin/hex/uf2 files
# target_compile_definitions(${TEST_JIG_FIRST_PHASE} PUBLIC NEURONWIRED=1 FMW_VERSION=${VERSION})
# pico_add_extra_outputs(${TEST_JIG_FIRST_PHASE})
# pico_set_linker_script(${TEST_JIG_FIRST_PHASE} ${CMAKE_CURRENT_BINARY_DIR}/memmap_mainProgram.ld)

# # Combine the bootloader and test jig into one flashable UF2 image
# ################################################################################

# set(COMPLETE_UF2_TEST_JIG ${CMAKE_CURRENT_BINARY_DIR}/TEST_JIG_FIRST_PHASE)

# find_package(Python3 REQUIRED COMPONENTS Interpreter)
# add_custom_command(OUTPUT ${COMPLETE_UF2_TEST_JIG} DEPENDS ${BOOTLOADER} ${TEST_JIG_FIRST_PHASE}
        # COMMENT "Building full UF2 image"
        # COMMAND ${Python3_EXECUTABLE}
        # ${CMAKE_CURRENT_SOURCE_DIR}/mergeBootloaderWithFirmware.py
        # -v ${VERSION}
        # -vs ${SEAL_VERSION}
        # -o ${COMPLETE_UF2_TEST_JIG}
        # ${BOOTLOADER} ${TEST_JIG_FIRST_PHASE}
        # )

# add_custom_target(${PROJECT} FIRST_PHASE_TEST_JIG DEPENDS ${COMPLETE_UF2_TEST_JIG})

# install(FILES ${COMPLETE_UF2_TEST_JIG} DESTINATION ${CMAKE_INSTALL_PREFIX})

# #End first phase
# ################################################################################


# #Second phase
# #####################################################################################

# set(TEST_JIG_SECOND_PHASE test_jig_second_phase)


# # Tell CMake where to find the executable source file
# add_executable(${TEST_JIG_SECOND_PHASE}
        # testjig_second_phase.cpp
        # )

# pico_generate_pio_header(${TEST_JIG_SECOND_PHASE} ${CMAKE_CURRENT_LIST_DIR}/headers/WS2812.pio)

# pico_set_program_name(${TEST_JIG_SECOND_PHASE} "Test Jig Second Phase")
# pico_set_program_version(${TEST_JIG_SECOND_PHASE} "0.1")

# # Enable usb output, disable uart output
# pico_enable_stdio_uart(${TEST_JIG_SECOND_PHASE} 0)
# pico_enable_stdio_usb(${TEST_JIG_SECOND_PHASE} 1)

# # Link to pico_stdlib (gpio, time, etc. functions)
# target_include_directories(${TEST_JIG_SECOND_PHASE} PRIVATE
        # headers
        # )

# # Link to pico_stdlib (gpio, time, etc. functions)
# target_link_libraries(${TEST_JIG_SECOND_PHASE}
        # pico_stdlib
        # pico_multicore
        # hardware_exception
        # hardware_dma
        # hardware_adc
        # hardware_flash
        # hardware_sleep
        # )

# # Add any user requested libraries
# target_link_libraries(${TEST_JIG_SECOND_PHASE}
        # WS2812
        # SPI
        # LEDTypes
        # IS31FL3743B
        # Debug_print
        # EEPROM
        # LEDManagement
        # LEDEffects
        # KeyScanner
        # Communications
        # RFCommunications
        # I2C
        # PinDeclarations
        # ModuleCommunications
        # )

# # Create map/bin/hex/uf2 files
# target_compile_definitions(${TEST_JIG_SECOND_PHASE} PUBLIC NEURONWIRED=1 FMW_VERSION=${VERSION})
# pico_add_extra_outputs(${TEST_JIG_SECOND_PHASE})
# pico_set_linker_script(${TEST_JIG_SECOND_PHASE} ${CMAKE_CURRENT_BINARY_DIR}/memmap_mainProgram.ld)

# # Combine the bootloader and test jig into one flashable UF2 image
# ################################################################################

# set(COMPLETE_UF2_TEST_JIG_SECOND ${CMAKE_CURRENT_BINARY_DIR}/TEST_JIG_SECOND_PHASE)

# find_package(Python3 REQUIRED COMPONENTS Interpreter)
# add_custom_command(OUTPUT ${COMPLETE_UF2_TEST_JIG_SECOND} DEPENDS ${BOOTLOADER} ${TEST_JIG_SECOND_PHASE}
        # COMMENT "Building full UF2 image"
        # COMMAND ${Python3_EXECUTABLE}
        # ${CMAKE_CURRENT_SOURCE_DIR}/mergeBootloaderWithFirmware.py
        # -v ${VERSION}
        # -vs ${SEAL_VERSION}
        # -o ${COMPLETE_UF2_TEST_JIG_SECOND}
        # ${BOOTLOADER} ${TEST_JIG_SECOND_PHASE}
        # )

# add_custom_target(${PROJECT} SECOND_PHASE_TEST_JIG DEPENDS ${COMPLETE_UF2_TEST_JIG_SECOND})

# install(FILES ${COMPLETE_UF2_TEST_JIG_SECOND} DESTINATION ${CMAKE_INSTALL_PREFIX})

# #End second phase testers
# ###############################################################################

# #Tester RF phase
# #####################################################################################

# set(TEST_JIG_RF test_jig_rf)


# # Tell CMake where to find the executable source file
# add_executable(${TEST_JIG_RF}
        # testjig_rf.cpp
        # )

# pico_generate_pio_header(${TEST_JIG_RF} ${CMAKE_CURRENT_LIST_DIR}/headers/WS2812.pio)

# pico_set_program_name(${TEST_JIG_RF} "Test Jig RF")
# pico_set_program_version(${TEST_JIG_RF} "0.1")

# # Enable usb output, disable uart output
# pico_enable_stdio_uart(${TEST_JIG_RF} 0)
# pico_enable_stdio_usb(${TEST_JIG_RF} 1)

# # Link to pico_stdlib (gpio, time, etc. functions)
# target_link_libraries(${TEST_JIG_RF}
        # pico_stdlib
        # pico_multicore
        # SPI
        # LEDTypes
        # IS31FL3743B
        # LEDManagement
        # LEDEffects
        # PinDeclarations
        # ModuleCommunications
        # EEPROM
        # )

# target_include_directories(${TEST_JIG_RF} PRIVATE
        # headers
        # )

# target_link_libraries(${TEST_JIG_RF}
        # pico_stdlib
        # pico_multicore
        # hardware_exception
        # hardware_dma
        # hardware_flash
        # hardware_sleep
        # )

# # Add any user requested libraries
# target_link_libraries(${TEST_JIG_RF}
        # hardware_dma
        # hardware_i2c
        # hardware_pwm
        # hardware_clocks
        # hardware_adc
        # WS2812
        # SPI
        # LEDTypes
        # IS31FL3743B
        # LEDManagement
        # LEDEffects
        # KeyScanner
        # Debug_print
        # Communications
        # RFCommunications
        # I2C
        # )
# # Create map/bin/hex/uf2 files
# pico_add_extra_outputs(${TEST_JIG_RF})
# target_compile_options(${TEST_JIG_RF} PRIVATE -O0)


# target_compile_definitions(${TEST_JIG_RF} PUBLIC
        # NEURONWIRED=1
        # FMW_VERSION=${VERSION}
        # SPI_PORT=spi1
        # PIN_MISO=8
        # PIN_SCK=14
        # PIN_MOSI=11
        # PIN_CSRF=9
        # PIN_SS=19
        # DBG_PRINT_ENABLED
        # DBG_PRINT_TRACE_ENABLED
        # UPGRADE_RF_TESTER_PROGRAM
        # )


# # Create map/bin/hex/uf2 files
# pico_add_extra_outputs(${TEST_JIG_RF})

# # Automatize RF Update program.
# ################################################################################

# add_custom_target(RF_UPDATE
        # COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/RFUpgrade.py
        # WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        # COMMENT "Running RF Upgrade Script"
# )

# #End rf phase
# ################################################################################