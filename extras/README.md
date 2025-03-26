# Extras

## BuzzFeed Utility
This utility consists of a pair of programs, buzzfeed.ino and buzzfeed.php, which together allow you to easily experiment with the BuzzKill board by sending data directly from your PC or laptop.

### buzzfeed.ino
This program runs on an Arduino and acts as a bridge to transfer the data bytes it receives from the PC/laptop to the BuzzKill board. Be sure to configure the options at the top of the code to use either SPI or I2C according to how your board is connected. Once uploaded to the Arduino, it will listen for incoming data through the USB port and relay that data directly to the BuzzKill board.

### buzzfeed.php
This PHP script runs on your PC/laptop. You will need a recent version of PHP installed, as well as the PHP DIO extension. On most systems you can run the script from the current directory by typing "php buzzfeed.php" or "\PHP\php.exe buzzfeed.php". If your PHP installation is in a different directory you may need to substitute the exact path to your php or php.exe command.

Run the script with no arguments to see the available options. You can supply data either in an immediate argument string, or stored in a file. The format is the same for either, except that files may also contain comments (see below).

### Data Format
The data is given as a series of hexadecimal bytes, 2 characters for each byte, with or without separating spaces. Upper or lowercase are acceptable. Line breaks are optional, with one exception: within a file, a semicolon (;) marks the beginning of a comment that continues until the end of the current line. Thus, for each semicolon character there must be a following linefeed character.

There is also a special "wait" command which may be used in place of a data byte. This should be included as the letters "WT" followed by two bytes representing the low and high bytes of a time value. This will cause a delay of the specified time before proceding with the reading of the following bytes. The time value should be given in milliseconds.

Here is an example file which may be used with the -f option:
```
; A "computer" type sound effect
40 04 40 1f 80 40
50 04 20 30 80 60
00 04 80 00 80 a0
10 04 60 00 80 a0
c8 04 0f 20 5f 50
8a ff 50
9a ff 20
c1 f3
WT 88 13 ; Pause for 5 seconds
00 00 ; Stop all sound
```

