# Firmware Notes

## Preparing the Firmware

A ready-to-use binary file [BuzzKill_firmware.hex](./BuzzKill_firmware.hex) is provided for quick installation. It should be suitable for nearly all users.

If you prefer, you can assemble your own binary from the [source files](./src/). They are available individually or all together as a .zip file. Be sure to look at the buildopts.inc file where you can easily change some of the program defaults to suit your needs. The easiest way to assemble from source is using Microchip Studio, which is available for free. Otherwise any IDE or utility that invokes the AVRASM2 assembler will work.

## Flashing the Firmware

The board utilizes a UPDI interface, requiring only one wire in addition to power and ground. If you're using a stand-alone programmer, it may provide its own power/ground outputs. Otherwise you will need to power the board separately. Just use the expected VCC and GND pads for this, on either the J3 or J4 connectors on the BuzzKill board. The UPDI line from the programmer should be connected to the SS pad (J2 pad 5) on the BuzzKill board.

The "official" programmer is the Atmel-ICE (or ATAMEL-ICE). This is what I used for all development on this project, but it has gotten quite expensive recently. Several cheaper options exist now but I can't personally attest to any of them. Anything that can handle AVR UPDI protocol should work, especially if AVRDD class is specifically listed as compatible.

## No Programmer, No Problem!

If you don't already have a compatible programmer, you may be able to use an Arduino instead. If you were already planning to connect the BuzzKill board to an Arduino, you may already have everything you need. I can't guarantee this method will work with all Arduino versions or any particular board you may have, but it has worked for me in testing with a standard Arduino Uno board.

You'll need to download and install the most recent version of AVRDUDE from <https://github.com/avrdudes/avrdude/releases>. If you already have it installed, make sure the version number is at least 8.0.

Connect BuzzKill VCC (J3/J4 pad 2) to 5V on the Arduino, and BuzzKill GND (J3/J4 pad 6) to GND on the Arduino. If you have mounted the BuzzKill board onto the Arduino ICSP header, these connections have been made automatically. Next connect BOTH the RX (pin 0) and TX (pin 1) on the Arduino to SS (J2 pad 5) on the BuzzKill. If you have the BuzzKill mounted on a breadboard, this will be simple. Otherwise you will need to improvise a basic "Y" connector of some kind. Anything that gets SS connected to both RX and TX will work. Finally on the Arduino you need to ground the RESET pin. A simple wire from RESET to any of the GND pins will do.

Now connect the Arduino to your computer with a USB cable and execute the following command:
```
/full/path/avrdude -v -p 16dd14 -c serialupdi -P COM1 -U flash:w:/full/path/BuzzKill_firmware.hex:i -b 230400
```

If your Arduino is not connected on COM1, make sure to substitute the actual port name. And use your actual filepaths for the avrdude executable and the .hex file.

There's a good chance you'll see an error message and the process will abort. If so, change the last number to 115200 and try again. As long as you get an error, keep adjusting the last number downward according to the following sequence:
```
230400, 115200, 57600, 28800, 14400, 9600, 4800
```

If you still get an error after trying 4800, remove and re-plug the USB cable, then try again from the start. Make sure to use the numbers in the given sequence, don't jump around arbitrarily or try the same number more than once per cycle. At some point you should see the flashing process begin, followed by a read verification, and a success message.

## Done!

Once you have flashed the binary image, your BuzzKill board is complete!
