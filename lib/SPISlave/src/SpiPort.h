/*
 * kaleidoscope::device::dygma::Wired -- Kaleidoscope device plugin for Dygma Wired
 *
 * Copyright (C) 2020-2023  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Mantainer: Gustavo Gomez Lopez @Noteolvides
 *  Mantainer: Juan Hauara @JuanHauara
 */

#ifndef SPICOMUNICATIONS_H_
#define SPICOMUNICATIONS_H_


#include "Spi_slave.h"
#include "Communications_protocol.h"

using namespace Communications_protocol;

#define SIDE_nRESET_1 22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2 10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset

#define SPI_SPEED     (4000 * 1000) // 4 MHz

// SPI0
#define SPI_PORT_0    spi0
#define PIN_MISO0         23
#define PIN_MOSI0         20
#define PIN_CLK0          18
#define PIN_CS0           21


// SPI1
#define SPI_PORT_1    spi1
#define PIN_MISO1   11
#define PIN_MOSI1   8
#define PIN_CLK1    14
#define PIN_CS1     9

// SPI2
#if 0

        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(0, 29)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#endif

// clang-format on

class SpiPort 
{
    public:
        SpiPort(uint8_t _spi_port_used);

        Spi_slave *spi_slave = nullptr;

        void init(void);
//        void deInit(void);
        void run(void);

        bool readPacket(Packet &packet);

        bool sendPacket(Packet &packet);

        void clearSend();
        void clearRead();


       private:
        uint8_t spi_port_used;
};


#endif //SPICOMUNICATIONS_H_
