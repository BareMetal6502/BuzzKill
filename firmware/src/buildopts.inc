/**********
*
* BuzzKill Sound Effects Board v1.0 for AVR16DD14 microcontroller
*
* Released under MIT License
*
* Copyright (c) 2025 Todd E. Stidham
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
*
**********/

/**********
* Here we define some options for changing the default program behavior
* We don't try to anticipate everything, just the most likely customization needs
**********/

; Protocol select
	.EQU PROTOCOL_SEL = 0	; Which type of protocol (SPI or TWI) to use. Options are:
			; 0: SPI by default, TWI if J1 is shorted
			; 1: TWI by default, SPI is J1 is shorted
			; 2: SPI always, regardless of J1
			; 3: TWI always, regardless of J1

; Automatic sleep
	.EQU SLEEP_AT_BOOT = 0	; If 1, automatically enter sleep mode after start-up

; TWI interface
	.EQU TWI_ADDR_DEF = 10	; The default TWI address, used unless a new one has been assigned by command
	.EQU TWI_EEPROM = 0x1400	; The location in EEPROM where we store the TWI address and checksum bytes
