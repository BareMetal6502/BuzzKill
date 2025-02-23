# Firmware Notes.

## Preparing the Firmware

A ready-to-use binary file [BuzzKill_firmware.hex](./BuzzKill_firmware.hex) is provided for quick installation. It should be suitable for nearly all users.

If you prefer, you can assemble your own binary from the [source files](./src/). They are available individually or all together as a .zip file. Be sure to look at the buildopts.inc file where you can easily change some of the program defaults to suit your needs. The easiest way to assemble from source is using Microchip Studio, which is available for free. Otherwise any IDE or utility that invokes the AVRASM2 assembler will work.

## Flashing the Firmware

The only essential requirement is an AVR-compatible UPDI programmer. The "official" programmer is the Atmel-ICE (or ATAMEL-ICE). This is what I used for all development in this project, but it has gotten quite expensive recently. Several cheaper option exist now but I can't personally attest to any of them. Anything that can handle AVR UPDI protocol should work, especially if AVRDD class is specifically listed.

The physical connection from the board to the programmer will usually only require three wires -- power, ground, and UPDI. Your programmer may provide the power to the board, or (as is the case with the Atmel-ICE) the board will need to be powered separately. Just use the expected VCC and GND pins for this, on either the J3 or J4 connectors on the BuzzKill board. The UPDI line should be connected to the SS pad on J2 on the BuzzKill board.

## Done!

Once you have flashed the binary image to the board, your BuzzKill board is complete!
