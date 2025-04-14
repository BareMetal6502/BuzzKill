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
* The general process for each phoneme is to use table lookups to determine the length of the phoneme, and the frequency and amplitude data for each of four formants
* For each formant adjustment factors will also be applied to scale the frequency and amplitude values according to user specification
* Then delta (step) values will be calculated so that frequencies and amplitudes will not change abruptly but will "glide" in steps from old values to new values
**********/

speech:	ldi YL, LOW(spchctrl)
	ldi YH, HIGH(spchctrl)
	sbic gpr_spch, 6
	rjmp speech2
	sbi gpr_spch, 6
	cbi gpr_spch, 2
	ldi work1, 52		; Insert an implicit short "space" (code 52) as first phoneme
	std Y+14, work1		; This allows the first real phoneme to be blended in from silence, instead of starting abruptly
	sbi VPORTD_OUT, 4		; Set SOUT pin high
	rjmp spchloada
speech2:	sbic gpr_spch, 5
	rjmp speech3
	sbi gpr_spch, 5
	rjmp spchadvnce
speech3:	ldd work1, Y+11		; Check if we are in an even or odd frame
	sbis gpr_spch, 2
	rjmp spcheven
spchodd:	cbi gpr_spch, 2
	sbis gpr_spch, 1
	rjmp spchdone
	cpi work1, 253 
	brlo spchdone_a
	breq spchloada
	cpi work1, 255
	breq spchloade
	sbis gpr_spch, 3
	rjmp spchloadc
	std Y+15, zero
spchdone_a:	ldi ZH, HIGH(ctrlgrid)
	jmp patches
		
spchloada:	ldd ZL, Y+14		; Load ZL with code of next phoneme
	clr ZH		; Phoneme codes are 8 bits so high byte is zero
	subi ZL, -LOW(tabplen<<1)	; Add memory offset of phoneme length table
	sbci ZH, -HIGH(tabplen<<1)	; Z now points to index of phoneme within table
	lpm work1, Z		; Load data from table
	std Y+15, work1 		; Store data as length of next phoneme
	rjmp spchdone
		
spchloadc:	ldd ZL, Y+14		; Load ZL with code of next phoneme
	lsl ZL		; Double the value since each table entry is two bytes
	ldi ZH, HIGH(tabform2<<1)	; Add memory offset of formant2 table
	lpm work1, Z+		; Load frequency data byte from table
	ldd work2, Y+3		; Load adjustment factor
	mul work1, work2		; Multiply table data by adjustment factor
	std Y+18, R0		; Store as 16-bit frequency
	std Y+19, R1
	ldi XL, LOW(oscctrl+20)	; Point X at frequency data of current phoneme
	ldi XH, HIGH(oscctrl)
	ld work1, X+		; Load current frequency low byte
	ld work2, X+		; Load current frequency high byte
	sub R0, work1		; Subtract current frequency from next frequency
	sbc R1, work2
	asr R1		; Divide difference by two to set delta size
	ror R0 
	ldd work3, Y+13		; Load length of current phoneme
	subi work3, 5		; Check if it's less than 5
	brlo spchldc2
	asr R1		; For longer lengths, divide by two again (reduce delta size)
	ror R0
spchldc2:	std Y+30, R0 		; Store as frequency delta
	std Y+31, R1
	ldi store4, 127
	lpm work1, Z		; Load amplitude data byte from table
	ldd work2, Y+4		; Load adjustment factor
	lsl work1
	mul work1, work2		; Multiply table data by adjustment factor
	sbrc R1, 7		; Need to cap result at 7 bits, so if high bit is set...
	mov R1, store4		; Replace high byte with 0b01111111
	std Y+25, R1		; Store as 8-bit amplitude
	subi XL, -58		; Point to envelope level
	ld work1, X+		; Load current amplitude
	sub R1, work1		; Subtract current amplitude from next amplitude
	asr R1		; Divide difference by two to set delta size
	sbrs work3, 7		; If current phoneme length is longer than 4...
	asr R1		; Divide by two again (reduce delta size)
	std Y+35, R1		; Store as amplitude delta
	rjmp spchdone

spchloade:	ldd ZL, Y+14		; Load ZL with code of next phoneme
	lsl ZL		; Double the value since each table entry is two bytes
	ldi ZH, HIGH(tabform4<<1)	; Add memory offset of formant4 table
	lpm work1, Z+		; Load frequency data byte from table
	ldd work2, Y+7		; Load adjustment factor
	mul work1, work2		; Multiply table data by adjustment factor
	std Y+22, R0		; Store as 16-bit frequency
	std Y+23, R1
	ldi store4, 127
	lpm work1, Z		; Load amplitude data byte from table
	ldd work2, Y+8		; Load adjustment factor
	lsl work1
	mul work1, work2		; Multiply table data by adjustment factor
	sbrc R1, 7		; Need to cap result at 7 bits, so if high bit is set...
	mov R1, store4		; Replace high byte with 0b01111111
	std Y+27, R1		; Store as 8-bit amplitude
	sbic gpr_spch, 4
	sbi gpr_spch, 3
	rjmp spchdone

spchloadb:	ldd ZL, Y+14
	lsl ZL
	ldi ZH, HIGH(tabform1<<1)
	lpm work1, Z+
	ldd work2, Y+1
	mul work1, work2
	std Y+16, R0
	std Y+17, R1
	ldi XL, LOW(oscctrl+16)
	ldi XH, HIGH(oscctrl)
	ld work1, X+
	ld work2, X+
	sub R0, work1
	sbc R1, work2
	asr R1
	ror R0
	ldd work3, Y+13
	subi work3, 5
	brlo spchldb2
	asr R1
	ror R0
spchldb2:	std Y+28, R0
	std Y+29, R1
	ldi store4, 127
	lpm work1, Z
	ldd work2, Y+2
	lsl work1
	mul work1, work2
	sbrc R1, 7
	mov R1, store4
	std Y+24, R1
	subi XL, -58
	ld work1, X
	sub R1, work1
	asr R1
	sbrs work3, 7
	asr R1
	std Y+34, R1
	rjmp spchdone

spchloadd:	ldd ZL, Y+14
	lsl ZL
	ldi ZH, HIGH(tabform3<<1)
	lpm work1, Z+
	ldd work2, Y+5
	mul work1, work2
	std Y+20, R0
	std Y+21, R1
	ldi XL, LOW(oscctrl+24)
	ldi XH, HIGH(oscctrl)
	ld work1, X+
	ld work2, X+
	sub R0, work1
	sbc R1, work2
	ldd work3, Y+13
	asr R1
	ror R0
	subi work3, 5
	brlo spchldd2
	asr R1
	ror R0
spchldd2:	std Y+32, R0
	std Y+33, R1
	ldi store4, 127
	lpm work1, Z
	ldd work2, Y+6
	lsl work1
	mul work1, work2
	sbrc R1, 7
	mov R1, store4
	std Y+26, R1
	subi XL, -58
	ld work1, X+
	sub R1, work1
	asr R1
	sbrs work3, 5
	asr R1
	std Y+36, R1
	rjmp spchdone
		
spchclear:	cbi VPORTD_OUT, 4		; Set SOUT pin low
	out gpr_spch, zero
	std Z+16, zero
	std Z+17, zero
	std Z+20, zero
	std Z+21, zero
	std Z+24, zero
	std Z+25, zero
	std Z+28, zero
	std Z+29, zero
	rjmp spchdone

spchtick:	cbi gpr_spch, 1
	ldd work1, Y+10
	dec work1
	std Y+10, work1
	brne spchtock
spchadvnce:	ldi ZL, LOW(oscctrl)
	ldi ZH, HIGH(oscctrl)
	ldd work1, Y+15
	tst work1
	breq spchclear
	ldd work1, Y+16
	std Z+16, work1
	ldd work1, Y+17
	std Z+17, work1
	ldd work1, Y+18
	std Z+20, work1
	ldd work1, Y+19
	std Z+21, work1
	ldd work1, Y+20
	std Z+24, work1
	ldd work1, Y+21
	std Z+25, work1
	ldd work1, Y+22
	std Z+28, work1
	ldd work1, Y+23
	std Z+29, work1
	ldi ZL, LOW(envdata)
	ldi ZH, HIGH(envdata)
	ldd work1, Y+24
	std Z+0, work1
	std Z+3, work1
	ldd work1, Y+25
	std Z+4, work1
	std Z+7, work1
	ldd work1, Y+26
	std Z+8, work1
	std Z+11, work1
	ldd work1, Y+27
	std Z+12, work1
	std Z+15, work1
	ldd work1, Y+14
	std Y+12, work1
	ldd work1, Y+15
	std Y+13, work1
	std Y+10, work1
	sbi gpr_spch, 1
spchtock:	ldd work1, Y+0
	std Y+11, work1
spchdone:	ldi ZH, HIGH(ctrlgrid)
  	jmp patches

spcheven:	sbi gpr_spch, 2
	ldd work1, Y+11
	inc work1
	std Y+11, work1
	breq spchtick
	sbic gpr_spch, 1
	rjmp spcheven2
	cpi work1, 254
	brlo spchevdn
	ldd work1, Y+10
	ldd work2, Y+13
	breq spchevena
spchevenb:	cpi work2, 3
	brlo spchevdn
	subi work1, 2
	breq spchstepb
	brmi spchevdn
	cpi work2, 5
	brlo spchevdn
	cpi work1, 3
	brsh spchevdn		; If lower fall through to spchstepb
spchstepb:	ldi ZL, LOW(envdata)
	ldi ZH, HIGH(envdata)
	ldd R1, Z+0
	ldd work2, Y+34
	add R1, work2
	std Z+0, R1
	std Z+3, R1
	ldd R1, Z+4
	ldd work2, Y+35
	add R1, work2
	std Z+4, R1
	std Z+7, R1
	ldd R1, Z+8
	ldd work2, Y+36
	add R1, work2
	std Z+8, R1
	std Z+11, R1
spchevdn:	ldi YH, HIGH(ctrlgrid)
	ldi ZH, HIGH(ctrlgrid)
	jmp patches

spchevena:	cpi work2, 3
	brlo spchevdn
	subi work1, 2
	breq spchstepa
	brmi spchevdn
	cpi work2, 6
	brlo spchevdn
	cpi work1, 3
	brsh spchevdn		; If lower fall through to spchstepa
spchstepa:	ldi ZL, LOW(oscctrl)
	ldi ZH, HIGH(oscctrl)
	ldd work1, Z+16
	ldd work2, Y+28
	add work1, work2
	std Z+16, work1
	ldd work1, Z+17
	ldd work2, Y+29
	adc work1, work2
	std Z+17, work1
	ldd work1, Z+20
	ldd work2, Y+30
	add work1, work2
	std Z+20, work1
	ldd work1, Z+21
	ldd work2, Y+31
	adc work1, work2
	std Z+21, work1
	ldd work1, Z+24
	ldd work2, Y+32
	add work1, work2
	std Z+24, work1
	ldd work1, Z+25
	ldd work2, Y+33
	adc work1, work2
	std Z+25, work1
  	rjmp spchdone
spcheven2:	cpi work1, 253
	brlo spchevdn
	breq spchprel
	sbrs work1, 0
	rjmp spchloadb
	rjmp spchloadd
spchprel:	ldd work1, Y+14		; Load current phoneme
	cpi work1, 64
	brsh spchpull
	cpi work1, 56
	brlo spchprel2
	subi work1, -10
	std Y+14, work1
	rjmp spchdone
spchprel2:	cpi work1, 42
	brlo spchprel3
	cpi work1, 52
	brsh spchpull
	subi work1, -14
	std Y+14, work1
	rjmp spchdone
spchprel3:	cpi work1, 8
	brsh spchpull
	cpi work1, 2
	brlo spchprel4
	andi work1, 1
	ori work1, 18
	std Y+14, work1
	rjmp spchdone
spchprel4:	ldi work1, 17
	std Y+14, work1
	rjmp spchdone
spchpull:	ldd XL, Y+9		; Load buffer index of current phoneme
	cp XL, sblast		; Check if we're at end of buffer
	brsh spchover
	ldi XH, HIGH(spchbuff)
	ld work1, X+		; Fetch next phoneme from buffer
	std Y+14, work1		; Make next buffer phoneme the current phoneme
	std Y+9, XL		; Advance buffer index
	rjmp spchdone
spchover:	sbi gpr_spch, 4		; Set status bit to indicate final phoneme
	ldi work1, 52		; Insert implicit short space (code 52) at end of sequence
	std Y+14, work1
	rjmp spchdone
