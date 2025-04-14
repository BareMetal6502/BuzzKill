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
*
* This code handles all incoming data from TWI or SPI connection
*
* In multi-byte data streams some context must be kept for properly handling each incoming byte.
* This is done with the following registers:
*   gpr_mode: this holds the current command mode; zero if awaiting a new command
*   gpr_reg:  this keeps track of the register number or buffer location to save the next byte to
*   datacnt:  this keeps track of the number of bytes left in the command stream
*
* gpr_mode stores a bitfield designed to make efficient changes from one mode to another by changing single bits:
*   00000000: No active mode, next byte will be interpreted as start of a new command
*   00100000: Speech buffer mode, unknown length; next byte will be interpreted as length
*   00110000: Speech buffer mode, known length; next byte will be interpreted as a data byte to store in speech buffer
*   01000000: Speech speed mode; next byte will be interpreted as new speech speed
*   01010000: Speech factor mode; next byte will be interpreted as a new speech factor
*   01100000: Wave storage mode; next byte will be interpreted as a data byte to store in custom wave table
*   01110000: New TWI address mode; next byte will be interpreted as a data byte to store in twiaddrbuff
*   10000000: Register store mode, unknown length; next byte will be interpreted as length
*   10010000: Register store mode, known length; next byte will be interpreted as a data byte to store in a register
*   10100000: Osc reset mask mode; next byte will be interpreted as a bitfield specifying oscillators to reset
* The lower nibble is not used and always set to zero, so that the processing routines for each mode can be easily
* laid out as a jump table with at least 16 bytes for each entry
*
**********/

	.MACRO interrupt
modefacts:	sbi gpr_mode, 4
	sbi gpr_mode, 6
	ldi datacnt, LOW(spchctrl+1)
	out gpr_reg, datacnt
	ldi datacnt, LOW(spchctrl+9)
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
modewave:	sbi gpr_mode, 5
	sbi gpr_mode, 6
	out gpr_reg, zero
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
modetwi:	sbi gpr_mode, 4
	sbi gpr_mode, 5
	sbi gpr_mode, 6
	ldi datacnt, LOW(twiaddrbuff)
	out gpr_reg, datacnt
	ldi datacnt, 3
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
modermask:	sbi gpr_mode, 5
	sbi gpr_mode, 7
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
setsleep:	out gpr_mode, zero
	ldi datacnt, LOW(gosleep)
	sts RAMEND, datacnt
	ldi datacnt, HIGH(gosleep)
	sts RAMEND-1, datacnt
	reti
modereg:	mov intreg, datacnt
	lsr intreg
	lsr intreg
	out gpr_reg, intreg
	sbi gpr_mode, 7
	andi datacnt, 3
	breq modereg2
	sbi gpr_mode, 4
modereg2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
modespec:	cpi datacnt, 248
	breq modermask
	cpi datacnt, 245
	breq modefacts
	brsh modespec2
	sbi gpr_mode, 6		; Command 244 (0xf4), set speech speed
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
modespec2:	cpi datacnt, 249
	breq modewave
	cpi datacnt, 250
	breq modetwi
	cpi datacnt, 251
	breq setsleep
	brsh modespec3
	sts spchctrl+9, zero	; Command 246-247 (0xf6-0xf7), start or stop speech
	bst datacnt, 0
	ldi datacnt, 128
	out gpr_spch, datacnt
	brts modespec3
	sts spchctrl+15, zero
	sbi gpr_spch, 6
modespec3:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti

	ALIGN 256		; We want the following code aligned on a page boundary to make an efficient calculated jump
	.SET handlerblock=PC>>8	; Note current code page, so we can compare to desired value and verify handler block location

modeless:	lds datacnt, @0		; mode=0
	cp datacnt, twoforty
	brlo modereg
modeless2:	cpi datacnt, 244
	brsh modespec
modespch:	out gpr_reg, zero
	sbi gpr_mode, 5
	andi datacnt, 3
	breq modespch2
	sbi gpr_mode, 4
modespch2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
rstspch:	mov sblast, zero
	sts spchctrl+9, zero
	out gpr_mode, zero
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 32
lenspch:	lds datacnt, @0		; mode=32
	tst datacnt
	breq rstspch
	sbi gpr_mode, 4
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
stspch:	lds intreg, @0		; mode=48
	mov ZL, sblast
	ldi ZH, HIGH(spchbuff)
	st Z+, intreg
	mov sblast, ZL
	dec datacnt
	brne stspch2
	out gpr_mode, zero
stspch2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
spspeed:	lds intreg, @0		; mode=64
	sts spchctrl, intreg
	out gpr_mode, zero
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
spfacts:	lds intreg, @0		; mode=80
	in ZL, gpr_reg
	ldi ZH, HIGH(spchctrl+1)
	st Z+, intreg
	out gpr_reg, ZL
	cp ZL, datacnt
	brcs spfacts2
	out gpr_mode, zero
spfacts2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
stwave:	lds intreg, @0		; mode=96
	in ZL, gpr_reg
	ldi ZH, HIGH(tabcust)
	st Z+, intreg
	out gpr_reg, ZL
	tst ZL
	brne stwave2
	out gpr_mode, zero
stwave2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
sttwiaddr:	lds intreg, @0		; mode=112
	in ZL, gpr_reg
	ldi ZH, HIGH(twiaddrbuff)
	st Z+, intreg
	out gpr_reg, ZL
	dec datacnt
	breq twiaddrrecv
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
lenreg:	lds datacnt, @0		; mode=128
	tst datacnt
	breq rstregs
	sbi gpr_mode, 4
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
streg:	lds intreg, @0		; mode=144
	in ZL, gpr_reg
	ldi ZH, HIGH(ctrlgrid)
	st Z+, intreg
	out gpr_reg, ZL
	dec datacnt
	brne streg2
	out gpr_mode, zero
streg2:	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
	ALIGN 16
strmask:	lds intreg, @0		; mode=160
	sts rstmask, intreg
	out gpr_mode, zero
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
rstregs:	movw saveptr, ZL
	in ZL, gpr_reg
	subi ZL, 60
	ldi ZH, HIGH(tabdflts)
rstregs2:	ldd intreg, Z+0
	std Z+60, intreg
	inc ZL
	brne rstregs2
	out gpr_mode, zero
	movw ZL, saveptr
	out CPU_SREG, savestat
	reti
twiaddrrecv:	out gpr_mode, zero
	ldi datacnt, LOW(newtwiaddr)
	sts RAMEND, datacnt
	ldi datacnt, HIGH(newtwiaddr)
	sts RAMEND-1, datacnt
	reti
	.ENDMACRO
