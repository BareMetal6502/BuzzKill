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
* This code will update a single oscillator. It is defined with macros so it can be easily repeated eight times.
*
* The code is divided into two separate macros, OSCILLATORA and OSCILLATORB.
* OSCILLATORA handles incrementing/decrementing the accumulator or resetting it, applying the midpoint adjustment,
* and jumping to the appropriate waveform handler in OSCILLATORB. It does not require any special alignment.
*
* OSCILLATORB calculates the specified waveform, stores the values, adjusts the volume, and adds to the global output.
* The waveform handlers are structured as a jump table, so the macro needs to be aligned on a page boundary.
*
**********/

	.MACRO OSCILLATORA
osclltr:	ldd store2, Y+2		; Get high byte of accumulator
	mov aux3, store2		; Save it for later comparison
	lsr aux1		; Rotate RST byte, bit for this osc is now in carry bit
	.IF @0>3		; If this is a voice osc...
	ldd aux2, Z+47		; we use the scratch control byte...
	bst aux2, 6		; and also check the RST bit there...
	brts oscrst		; and reset the osc if it's set
	.ELSE
	ldd aux2, Z+3		; If this is not a voice osc, use the register control byte
	.ENDIF
	brcs oscrst		; If RST bit (in carry) was set, reset the osc
	.IF @0>3		; If this is a voice osc, we'll get the freq from the scratch bytes
	ldd store3, Z+44
	ldd store4, Z+45
	.ELSE		; Otherwise we'll get the freq straight from the registers
	ldd store3, Z+0
	ldd store4, Z+1
	.ENDIF		; Freq is now in store3/store4
	bst aux2, 4		; Check the REV bit from control byte
	brts steprev		; If set we step in reverse, otherwise we step forward
stepfwd:	ldd work1, Y+0
	add work1, store3
	std Y+0, work1
	ldd store1, Y+1
	adc store1, store4
	std Y+1, store1
	adc store2, zero
	rjmp stepall
steprev:	ldd work1, Y+0
	sub work1, store3
	std Y+0, work1
	ldd store1, Y+1
	sbc store1, store4
	std Y+1, store1
	sbci store2, 0
stepall:	mov store5, store2		; Save the new accumulator top byte for later comparison
	andi store2, 3		; Only need the bottom two bits (18 bits total)
	std Y+2, store2		; Save trimmed byte to accumulator
stepred:	lsr store2		; The upper 8 bits of the accumulator are the osc raw count value
	ror store1		; We need to shift twice to get the top 8 bits into one byte
	lsr store2
	ror store1		; store1 has the raw count value, now need to adjust for midpoint
	.IF @0>3		; For voice osc, midpoint value is in scratch byte
	ldd XL, Z+46
	.ELSE		; Otherwise it's in the register byte
	ldd XL, Z+2
	.ENDIF		; Either way now midpoint value is in XL
	ldi XH, HIGH(tabmidl)	; We'll use it as an index into midl table
	cp store1, XL		; Check if the osc count has reached the midpoint
	brsh osceast		; If yes we are in "east" sector, otherwise in "west"
oscwest:	ld work1, X
	mul work1, store1
	mov work2, R1
	inc XH
	ld work1, X
	mul work1, store1
	add work2, R0
	rjmp oscidx
osceast:	sub store1, XL
	com XL
	ld work1, X
	mul work1, store1
	mov work2, R1
	inc XH
	ld work1, X
	mul work1, store1
	add work2, R0
	ori work2, 128
oscidx:	movw XL, ZL		; work2 now contains adjusted count
	ldd ZL, Z+3		; Get osc control byte
	andi ZL, 0b11100000		; Waveform is determined by bits 5-7
	.SET oscpagetarg=(PC>>8)+1	; Calculate the code page where the waveform handlers will reside
	ldi ZH, oscpagetarg		; ZH points to the code page, ZL points to the 32-byte block within the page
	ijmp		; Jump to the specific code handler for the selected waveform
oscrst:	ldi store1, 128		; Reset the osc by clearing the accumulator and setting output to neutral
	std Y+6, store1
	clr store1
	clr store2
	clr aux3
	std Y+0, store1
	std Y+1, store2
	rjmp stepall
	.ENDMACRO

	.MACRO OSCILLATORB		; Upon entry work2 contains the adjusted osc count, we replace it with the waveform output
	ALIGN 256
	.SET oscpagedest=PC>>8	; Find current code page, so we can compare to predicted value and verify jump table location
wfsine:	mov ZL, work2		; Waveform 0, sine wave
	ldi ZH, HIGH(tabsin<<1)	; Use lookup table to convert count to sine[count]
	lpm work2, Z
	rjmp wfdone
	ALIGN 32
wfsaw:	rjmp wfdone		; Waveform 1, sawtooth/ramp, work2 already correct so do nothing
	ALIGN 32
wftri:	lsl work2		; Waveform 2, triangle wave
	brcc wftri2
	com work2
wftri2:	rjmp wfdone
	ALIGN 32
wfpulse:	bst work2, 7		; Waveform 3, pulse wave
	brts wfpulse2
	ser work2
	rjmp wfdone
wfpulse2:	clr work2
	rjmp wfdone
	ALIGN 32
wfexp:	mul work2, work2		; Waveform 4, exponential wave
	clr work2
	adc work2, R1
	rjmp wfdone
	ALIGN 32
wfnoise:	mov work1, store5		; Waveform 5, noise wave
	andi work1, 3
	cpse aux3, work1
	std Y+6, shiftl
	ldd work2, Y+6
	rjmp wfdone
	ALIGN 32
wfcust:	mov ZL, work2		; Waveform 6, custom wavetable
	ldi ZH, HIGH(tabcust)
	ld work2, Z
	rjmp wfdone
	ALIGN 32
wfhill:	mov ZL, work2		; Waveform 7, hilltop wave
	lsr ZL
	ldi ZH, HIGH(tabsin<<1)	; Use sine table with doubled count
	lpm work2, Z
	subi work2, 128		; Re-scale output from 128-255 to 0-254
	lsl work2
wfdone:	movw ZL, XL		; Restore saved Z pointer
	mov XL, aux2		; Assume tabstep is on 256-byte boundary
	andi XL, 7
	ldi XH, HIGH(tabstep)
	ld store1, X
	and work2, store1
	sbrc aux2, 3		; Check INV bit
	com work2		; Invert waveform if needed
	ldd store1, Y+3		; Determine crosses
	andi store1, 128
	bst work2, 7
	bld store5, 7
	asr store5
	eor store1, store5		; Bit 7=cross occurred, bit 6=new polarity, bit 1=accum overflowed
oscproc:	std Y+5, store1		; Store cross
	std Y+3, work2		; Store osc output (unsigned)
	subi work2, 128		; Convert to signed value
	std Y+4, work2		; Store osc output (signed)
	.IF @0>3
	lsr work3		; Rotate voice enable bits, current osc bit now in carry
	brcc oscdone		; Skip adding to output if disabled
	sbrc aux2, 5		; Check if a ring value is stored
	ldd work2, Y+7		; If ring value is present, use it instead
oscadd:	ldd work1, Z+63		; Load current envelope amplitude
	mulsu work2, work1
	movw store2, R0		; store2/store3=amped level, 15 bits signed
	ldd work1, Z+19		; Load envelope mix volume
	swap work1		; Mix volume is in bits 4-7
	andi work1, 15		; Isolate mix volume bits
	inc work1		; Mix volume now 1-16
	mul store2, work1
	movw store4, R0
	mulsu store3, work1
	add store5, R0
	adc R1, zero		; Result in store4/store5/R1, 19 bits signed
	swap R1
	swap store5
	swap store4
	andi store4, 15
	mov work1, store5
	and work1, twoforty
	or store4, work1
	andi store5, 15
	and R1, twoforty
	or store5, R1		; Now in store4/store5, 15 bits signed
	add outputl, store4		; Add to overall output
	adc outputh, store5
	.ENDIF
oscdone:
	.IF @0<7		; If not last oscillator, adjust pointers for next iteration
	subi ZL, -4
	subi YL, -8
	.ENDIF
	.ENDMACRO
