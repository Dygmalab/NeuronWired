const fs = require("fs");
const {crc32} = require("easy-crc");
const {SerialPort} = require("serialport");


async function init() {
    const hexFile = fs.readFileSync('KeyScanner_WithHeader.bin', "hex")
    const fromHexString = (hexString) => Uint8Array.from(hexString.match(/.{1,2}/g).map((byte) => parseInt(byte, 16)));
    const binaryFile = fromHexString(hexFile);
    const serialport = new SerialPort({path: '/dev/ttyACM1', baudRate: 115200})
    serialport.on('readable', function () {
        console.log('Data:', serialport.read().toString()
        )
    })

    serialport.write("upgrade.keyscanner.start\n")
    console.log("You have 4 seconds to press the button!")
    await sleep(4000);
    serialport.write("upgrade.keyscanner.beginLeft\n")
    await sleep(100);

    serialport.write("upgrade.keyscanner.getInfo\n")
    await sleep(100);

    let info = {
        hardwareVersion: 1,
        flashStart: 20480,
        programVersion: 16777217,
        programCrc: 3782824883
    }
    for (let i = 0; i < binaryFile.length; i=i+256) {
        serialport.write("upgrade.keyscanner.sendWrite ")
        const writeAction = new Uint8Array(new Uint32Array([info.flashStart+i,256]).buffer)
        const data = binaryFile.slice(i,i+256);
        const crc = new Uint8Array(new Uint32Array([crc32("CRC-32",data)]).buffer)
        const blob =  new Uint8Array(writeAction.length+data.length+crc.length)
        blob.set(writeAction)
        blob.set(data,writeAction.length)
        blob.set(crc,data.length+writeAction.length)
        const buffer = new Buffer.from(blob);
        serialport.write(buffer)
        await sleep(150);
    }

    serialport.write("upgrade.keyscanner.finish\n")

    process.exit(1);
}

function sleep(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}


init()