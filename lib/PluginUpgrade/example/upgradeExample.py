import struct
import time
import zlib

import numpy as np
import serial
from typing import NamedTuple


def padarray(A, size):
    t = size - len(A)
    return np.pad(A, pad_width=(0, t), mode='constant')


class InfoAction(NamedTuple):
    hardwareVersion: int
    flashStart: int
    programVersion: int
    programCrc: int


class SealHeader(NamedTuple):
    deviceId: int
    version: int
    size: int
    crc: int


class Seal(NamedTuple):
    hardwareVersion: SealHeader
    programStart: int
    programSize: int
    programCrc: int
    programVersion: int


def checkError(line):
    if line == b"false \r\n":
        print("Error something happened")
        exit(255)
    pass


def flash():
    # Lest get the info
    ser.write(b"upgrade.keyscanner.getInfo\n")
    line = ser.readline()
    ser.readline()
    checkError(line)
    line = line.replace(b" \r\n", b"")
    line = line.split(b" ")
    info: InfoAction = InfoAction(
        int(line[0]),
        int(line[1]),
        int(line[2]),
        int(line[3]),
    )
    print("This is the info")
    print(info)
    # Here we can check the version and crc
    program = open("KeyScanner_WithHeader.bin", mode='rb').read()
    # The first 32 bits are the seal.
    sealInBinary = struct.unpack(b"<IIIIIIII", program[0:32])
    sealInBinary = Seal(
        SealHeader(sealInBinary[0], sealInBinary[1], sealInBinary[2], sealInBinary[3]),
        sealInBinary[4], sealInBinary[5], sealInBinary[6], sealInBinary[7])
    print(sealInBinary)
    if sealInBinary.programCrc == info.programCrc:
        print("No need to update!")
        # exit(0)
    for i in range(0, len(program), 256):
        print("Writing " + str(i) + " of " + str(len(program)))

        write_format = "upgrade.keyscanner.sendWrite "
        writeAction = struct.pack(b"<II", i + info.flashStart, 256)
        ser.write(str.encode(write_format))
        ser.write(writeAction)
        chunk = program[i:(i + 256)]
        crc32Chunk = zlib.crc32(chunk)
        crc32Chunk = struct.pack(b"<I", crc32Chunk)
        ser.write(chunk)
        ser.write(crc32Chunk)
        line = ser.readline()
        checkError(line)
        ser.readline()

    ser.write(b"upgrade.keyscanner.finish\n")
    ser.readline()
    ser.readline()


def main():
    global ser
    # configure the serial connections (the parameters differs on the device you are connecting to)
    ser = serial.Serial(
        port='/dev/ttyACM1',
        baudrate=115200,
    )
    ser.isOpen()
    ser.write(b"upgrade.start\n")
    ser.readline()
    ser.readline()
    while True:
        ser.write(b"upgrade.isReady\n")
        line1 = ser.readline()

        ser.readline()

        if line1 == b"true \r\n":
            break
        time.sleep(1)

    print("Can start upgrading")

    ser.write(b"upgrade.keyscanner.beginLeft\n")
    line = ser.readline()
    checkError(line)
    ser.readline()
    flash()

    ser.write(b"upgrade.keyscanner.beginRight\n")
    line = ser.readline()
    checkError(line)
    ser.readline()
    flash()


main()