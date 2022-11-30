/* -*- mode: c++ -*-
 * kaleidoscope::util::flasher::KeyboardioI2CBootloader -- Flasher for Keyboardio's I2C Bootloader
 * Copyright (C) 2019  Keyboard.io, Inc
 * Copyright (C) 2019  Dygma Lab S.L.
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
 */

#pragma once

// TODO(@algernon): We should support AVR here, too.
#if defined(__SAMD21G18A__) || defined(__RP2040__)

#include <Wire.h>
#include "hardware/dma.h"
#include "kaleidoscope/util/flasher/Base.h"
#include "kaleidoscope/util/crc16.h"
#include "hardware/watchdog.h"

#define WIRE_ Wire1

#define DEBUG_BOOTLOADER
#ifdef DEBUG_BOOTLOADER
#define DBG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...) { }
#endif

namespace kaleidoscope {
namespace util {
namespace flasher {

template<typename _Props>
class KeyboardioI2CBootloader : kaleidoscope::util::flasher::Base<_Props> {
 public:
  static bool get_info_program(uint8_t address) {
	if (send_command(address, Action::INFO)) return false;
	read_data(address, (uint8_t *)&infoAction, sizeof(infoAction));

	DBG_PRINTF("ReadAction received with:\n");
	DBG_PRINTF("flashStart %lu:\n", infoAction.flashStart);
	DBG_PRINTF("validationHeader %lu:\n", infoAction.validationHeader);
	DBG_PRINTF("flashSize %lu:\n", infoAction.flashSize);
	DBG_PRINTF("eraseAlignment %lu:\n", infoAction.eraseAligment);
	DBG_PRINTF("writeAlignment %lu:\n", infoAction.writeAligment);
	DBG_PRINTF("maxDataLength %lu:\n", infoAction.maxDataLength);
	return true;
  }

  template<typename T>
  static bool verify(uint8_t address, T &firmware) {
	SealAction sealAction;
	if (read_validation_header(address, sealAction)) {

	  DBG_PRINTF("Keyscaner version %lu\n", sealAction.version);
	  DBG_PRINTF("Neuron Keyscnaer version %lu\n", firmware.KEY_SCANNER_VERSION);
	  DBG_PRINTF("Neuron CRC %lu\n", crc32(firmware.data, firmware.size));
	  DBG_PRINTF("Keyscane CRC  %lu\n", sealAction.crc);

	  return sealAction.version == firmware.KEY_SCANNER_VERSION
		  && crc32(firmware.data, firmware.size) == sealAction.crc;
	}
	return false;
  }

  template<typename T>
  static bool flash(uint8_t address, T &firmware) {
	//if (verify(address, firmware)) {
    // return true;
	//}

	if (!erase_program(address, firmware)) {
	  return false;
	}

	if (!write_firmware(address, firmware)) {
	  return false;
	}

	if (!verify_firmware(address, firmware)) {
	  return false;
	}

	return true;
  }

  static bool start_main_program(uint8_t address) {
	DBG_PRINTF("Sending GO\n");

	if (send_command(address, Action::GO)) return false;

	uint32_t vtor = infoAction.flashStart;
	if (send_message(address, (uint8_t *)&vtor, sizeof(vtor))) return false;

	return true;
  }

 private:
  struct CRCAndVersion {
	uint32_t version;
	uint32_t crc;
  };

  struct WriteAction {
	uint32_t addr;
	uint32_t size;
  };
  struct ReadAction {
	uint32_t addr;
	uint32_t size;
  };
  struct EraseAction {
	uint32_t addr;
	uint32_t size;
  };
  struct SealAction {
	uint32_t vtor;
	uint32_t size;
	uint32_t crc;
	uint32_t version;
  };

  struct InfoAction {
	uint32_t flashStart;
	uint32_t validationHeader;
	uint32_t flashSize;
	uint32_t eraseAligment;
	uint32_t writeAligment;
	uint32_t maxDataLength;
  };

  enum Action {
	INFO = 'I',
	WRITE = 'W',
	READ = 'R',
	ERASE = 'E',
	SEAL = 'S',
	GO = 'G',
  };

  inline static InfoAction infoAction;

  template<typename T>
  static bool write_firmware(uint8_t address, T &firmware) {
	uint8_t count;
	DBG_PRINTF("Starting to write all necessary blocks:\n");

	int32_t i = 0;
	for (uint32_t start = 0; start < firmware.size; start += 256) {
	  watchdog_update();
	  if (send_command(address, Action::WRITE)) return false;

	  //Limited by buffer of I2c :(
	  uint8_t data[256]{};
	  memcpy(data,
			 &firmware.data[start],
			 start + 256 < firmware.size ? sizeof(data) :
				 firmware.size % sizeof(data));

	  uint32_t calcCrc32ofData = crc32(data, sizeof(data));

	  WriteAction writeAction{infoAction.flashStart + start, sizeof(data)};
	  if (send_message(address, (uint8_t *)&writeAction, sizeof(writeAction))) return false;

	  if (send_message(address, (uint8_t *)&data, sizeof(data))) return false;

	  uint32_t crcSlave = 0;
	  read_data(address, (uint8_t *)&crcSlave, sizeof(crcSlave));

	  DBG_PRINTF("Written block %lu with address %lu and CRC is %i \n",
				 i++,
				 infoAction.flashStart + start,
				 calcCrc32ofData == crcSlave);

	  if (calcCrc32ofData != crcSlave) return false;
	}
	return true;
  }

  template<typename T>
  static bool erase_program(uint8_t address, T &firmware) {
	DBG_PRINTF("Starting to erase all necessary blocks:\n");
	uint32_t eraseLength = align(firmware.size, infoAction.eraseAligment);
	int32_t i = 0;
	for (uint32_t start = 0; start < eraseLength; start += infoAction.eraseAligment) {
	  watchdog_update();
	  if (send_command(address, Action::ERASE)) return false;

	  EraseAction eraseAction{infoAction.flashStart + start, infoAction.eraseAligment};
	  if (send_message(address, (uint8_t *)&eraseAction, sizeof(eraseAction))) return false;

	  DBG_PRINTF("Erasing block %lu with address %lu\n", i++, infoAction.flashStart + start);
	  uint8_t done = false;
	  read_data(address, (uint8_t *)&done, sizeof(done));

	  if (done == true) { DBG_PRINTF("Erased block %lu\n", i++); }
	  else { return false; }
	}
	return true;
  }

  template<typename T>
  static bool verify_firmware(uint8_t address, T &firmware) {
	DBG_PRINTF("Going to send Seal\n");

	if (send_command(address, Action::SEAL)) return false;

	SealAction sealAction;
	sealAction.vtor = infoAction.flashStart;
	sealAction.size = firmware.size;
	sealAction.crc = crc32(firmware.data, firmware.size);
	sealAction.version = firmware.KEY_SCANNER_VERSION;

	DBG_PRINTF("Going to send seal vtor %lu, size %lu, crc %lu and version %lu\n",
			   sealAction.vtor,
			   sealAction.size,
			   sealAction.crc,
			   sealAction.version);

	if (send_message(address, (uint8_t *)&sealAction, sizeof(sealAction))) return false;

	uint8_t done = false;
	read_data(address, (uint8_t *)&done, sizeof(done));
	return done;
  }

  static bool read_validation_header(uint8_t address, SealAction &sealAction) {
	DBG_PRINTF("Going to send Read\n");
	if (send_command(address, Action::READ)) return false;

	ReadAction readAction{infoAction.validationHeader, sizeof(SealAction)};
	if (send_message(address, (uint8_t *)&readAction, sizeof(readAction))) return false;

	read_data(address, (uint8_t *)&sealAction, sizeof(sealAction));
	DBG_PRINTF("Validation of the seal vtor %lu, size %lu, crc %lu and version %lu\n",
			   sealAction.vtor,
			   sealAction.size,
			   sealAction.crc,
			   sealAction.version);

	return true;
  }

  // Errors:
  //  0 : Success
  //  1 : Data too long
  //  2 : NACK on transmit of address
  //  3 : NACK on transmit of data
  //  4 : Other error
  static uint8_t send_command(uint8_t address, uint8_t command, bool stopBit = false) {
	WIRE_.beginTransmission(address);
	WIRE_.write(command);
	return WIRE_.endTransmission(stopBit);
  }

  static uint8_t send_message(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit = false) {
	WIRE_.beginTransmission(address);
	WIRE_.write(message, lenMessage);
	return WIRE_.endTransmission(stopBit);
  }

  static uint8_t read_data(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit = false) {
	WIRE_.requestFrom(address, lenMessage, false);
	return WIRE_.readBytes(message, lenMessage);
  }

  static uint32_t align(uint32_t val, uint32_t to) {
	uint32_t r = val % to;
	return r ? val + (to - r) : val;
  }

  //TODO: Change CRC32 Implementation for DMA
  static uint32_t crc32(const uint8_t *buf, size_t len) {
	uint32_t crc;
	static uint32_t table[256];
	static int have_table = 0;
	uint32_t rem;
	uint8_t octet;
	int i, j;
	const uint8_t *p, *q;

	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
	  /* Calculate CRC table. */
	  for (i = 0; i < 256; i++) {
		rem = i;  /* remainder from polynomial division */
		for (j = 0; j < 8; j++) {
		  if (rem & 1) {
			rem >>= 1;
			rem ^= 0xedb88320;
		  } else
			rem >>= 1;
		}
		table[i] = rem;
	  }
	  have_table = 1;
	}

	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
	  octet = *p;  /* Cast to unsigned octet. */
	  crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
  }

};

}
}
}

#endif
