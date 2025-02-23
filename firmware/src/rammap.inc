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
* Here we define the layout of our RAM space
* Some areas need to be aligned on certain boundaries, so there's some padding in the middle
* The padding area is called "heap" to identify it as useable scratch area if needed
**********/

spchbuff:	.BYTE	256	; Buffer holding speech phonemes
tabmidl:	.BYTE	256	; Table of midpoint adjustments (low byte)
tabmidh:	.BYTE	256	; Table of midpoint adjustments (high byte)
tabexpon:	.BYTE	256	; Table of exponent values
tabsynch:	.BYTE	256	; Table of frame synch values
tabcust:	.BYTE	256	; Table of custom waveform values
tabstep:	.BYTE	8	; Table of masks for waveform step function
spchctrl:	.BYTE	37	; Speech control parameters and data
twiaddress:	.BYTE	1	; TWI interface bus address
twiaddrbuff:	.BYTE	3	; Buffer for TWI address and checksum bytes
rstmask:	.BYTE	1	; Bitmask indicator for oscillator resets
heap:	.BYTE	146	; Unassigned space
tabdflts:	.BYTE	60	; Table of default register values
ctrlgrid:			; Beginning of register/control space, broken out below
oscctrl:	.BYTE	32	; Oscillator control registers
envctrl:	.BYTE	16	; Envelope control registers
globctrl:	.BYTE	2	; Global control registers
ptchctrl:	.BYTE	10	; Patch control registers
scratch:	.BYTE	16	; Scratch space for applying patch mods
envdata:	.BYTE	16	; Envelope status data
oscdata:	.BYTE	64	; Oscillator status data
