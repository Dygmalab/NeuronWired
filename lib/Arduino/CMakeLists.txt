add_library(Arduino INTERFACE)

target_compile_definitions(Arduino INTERFACE
        -DARDUINO_RASPBERRY_PI_PICO
        -DARDUINO_ARCH_RP2040
        -DARDUINO=10804
        -DCFG_DEBUG=0
        -DLFS_NO_ASSERT
        -DUSE_LFXO
        -DUSBCON
        -DUSE_TINYUSB
        )

target_include_directories(Arduino
        INTERFACE
		cores/rp2040
		# cores/rp2040/api
		# cores/rp2040/api/deprecated-avr-comp
		include
		include/pico_base
		libraries/Adafruit_TinyUSB_Arduino/src
		libraries/Adafruit_TinyUSB_Arduino/src/arduino
		libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/
		libraries/Wire/src
		variants/rpipico
		
		${PICO_SDK_PATH}/lib/tinyusb/src/
		${PICO_SDK_PATH}/src/boards/include
		${PICO_SDK_PATH}/src/common/pico_base/include
		${PICO_SDK_PATH}/src/common/pico_binary_info/include
		${PICO_SDK_PATH}/src/common/pico_bit_ops/include
		${PICO_SDK_PATH}/src/common/pico_divider/include
		${PICO_SDK_PATH}/src/common/pico_stdlib/include
		${PICO_SDK_PATH}/src/common/pico_sync/include
		${PICO_SDK_PATH}/src/common/pico_time/include
		${PICO_SDK_PATH}/src/common/pico_usb_reset_interface/include
		${PICO_SDK_PATH}/src/common/pico_util/include
		${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
		${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
		${PICO_SDK_PATH}/src/rp2_common/cmsis/include
		${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Core/Include
		${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Device/RaspberryPi/RP2040/Include
		${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_claim/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_clocks/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_divider/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_flash/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_interp/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
		${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_pll/include
		${PICO_SDK_PATH}/src/rp2_common/hardware_pwm/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_resets/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_vreg/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_watchdog/include
		# ${PICO_SDK_PATH}/src/rp2_common/hardware_xosc/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_async_context/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_bootrom/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_btstack/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_driver/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_double/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_float/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_int64_ops/include
		${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_multicore/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_platform/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_printf/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_runtime/include
		 ${PICO_SDK_PATH}/src/rp2_common/pico_rand/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_stdio/include
		# ${PICO_SDK_PATH}/src/rp2_common/pico_stdio_uart/include
		${PICO_SDK_PATH}/src/rp2_common/pico_unique_id/include
		${PICO_SDK_PATH}/lib/cyw43-driver/src
		${PICO_SDK_PATH}/lib/lwip/src/include
		${PICO_SDK_PATH}/lib/btstack/src
		${PICO_SDK_PATH}/lib/btstack/platform/embedded
        )

target_sources(Arduino
        INTERFACE
        cores/rp2040/api/Print.cpp
        cores/rp2040/api/Stream.cpp
        cores/rp2040/api/String.cpp
        cores/rp2040/_freertos.cpp
        cores/rp2040/CoreMutex.cpp
        cores/rp2040/delay.cpp
        cores/rp2040/main.cpp
        cores/rp2040/PIOProgram.cpp
        cores/rp2040/RP2040USB.cpp
        cores/rp2040/wiring_analog.cpp
        cores/rp2040/wiring_digital.cpp
        cores/rp2040/wiring_private.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_TinyUSB_API.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_CDC.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBH_Host.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/hid/Adafruit_USBD_HID.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/msc/Adafruit_USBD_MSC.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp
        libraries/Adafruit_TinyUSB_Arduino/src/class/cdc/cdc_device.c
        libraries/Adafruit_TinyUSB_Arduino/src/class/hid/hid_device.c
        libraries/Adafruit_TinyUSB_Arduino/src/class/midi/midi_device.c
        libraries/Adafruit_TinyUSB_Arduino/src/class/msc/msc_device.c
        libraries/Adafruit_TinyUSB_Arduino/src/class/vendor/vendor_device.c
        libraries/Adafruit_TinyUSB_Arduino/src/common/tusb_fifo.c
        libraries/Adafruit_TinyUSB_Arduino/src/device/usbd.c
        libraries/Adafruit_TinyUSB_Arduino/src/device/usbd_control.c
        libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/dcd_rp2040.c
        libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/rp2040_usb.c
        libraries/Adafruit_TinyUSB_Arduino/src/tusb.c
        libraries/Wire/src/Wire.cpp
        )

target_link_libraries(Arduino
        INTERFACE
        )
