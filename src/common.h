/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __COMMON_H__
#define __COMMON_H__


#include "stdint.h"

#define SPI_SPEED     (4000 * 1000) // 4 MHz

// SPI0
#define SPI_PORT_0 spi0
#define PIN_MISO0  23
#define PIN_MOSI0  20
#define PIN_CLK0   18
#define PIN_CS0    21


// SPI1
#define SPI_PORT_1          spi1
#define PIN_MISO1           11
#define PIN_MOSI1           8
#define PIN_CLK1            14
#define PIN_CS1             9

#define I2C_SDA_PIN         26  // SWe 20220719: I2C1 data out-/in-put, MASTER role
#define I2C_SCL_PIN         27  // SWe 20220719: I2C1 clock output, MASTER role
#define WIRE_               Wire1
#define I2C_CLOCK_KHZ       100
#define I2C_FLASH_CLOCK_KHZ 100  // flashing doesn't work reliably at higher clock speeds
//#define SIDE_POWER 1  // side power switch pa10; SWe 20220719: old, used in Neuron
#define SIDE_nRESET_1 22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2 10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset
#define nPWR_OK       1   // SWe 20220719: Power nOK IN-PULLUP from the 3.3V LDO, open drain, needs internal pull-up. NOTE: this is not implemented in the Development Board, only in the real WIRED Neuron2.
// SWe 20220719: LED pins
#define RGBW_LED_RED   6  // SWe 20220719: RED RGBW led OUT, PWM3 A can be used to control its intensity
#define RGBW_LED_GREEN 0  // SWe 20220719: GREEN RGBW led OUT, PWM0 A can be used to control its intensity
#define RGBW_LED_BLUE  2  // SWe 20220719: BLUE RGBW led OUT, PWM1 A can be used to control its intensity
#define RGBW_LED_WHITE 4  // SWe 20220719: WHITE RGBW led OUT, PWM2 A can be used to control its intensity
// SWe 20220719: analog pins
#define USB_CC1 28  // SWe 20220719: USB CC1 pin, can be used to check how much power the host does support by checking its analog value
#define USB_CC2 29  // SWe 20220719: USB CC2 pin, can be used to check how much power the host does support by checking its analog value
// SWe 20220719: ADC Vref input, tied to 3.3V with resistor and capacitor for filtering and buffering
// SWe 20220719: optional pins


#endif
