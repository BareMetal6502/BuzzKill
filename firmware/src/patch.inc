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
* Each patch consists of two bytes
* The first byte contains the SOURCE osc number (mod osc 0-3) and DESTINATION osc number (voice osc 0-3), and the patch type (0-15)
* The second byte is an optional parameter, its specific usage varies by the patch type
*
* Generally each patch type will read some data from the output area associated with the SOURCE mod oscillator
* It will process the data according to the patch type, possibly with additional values from the parameter or the DESTINATION osc
* Finally it will modify certain values in the scratch area associated with the DESTINATION voice oscillator
*
* Each PATCH macro totals exactly 256 words of code so they can be consecutively placed and maintain alignment
* It is subdivided into 16 sections using 16 words each, the first section is setup and the other 15 each process a specific patch type
* The alignment is based on placing the first jump target (patch0) at a page boundary (xx00 address)
* Thus patch1 will fall at xx10, patch2 at xx20, patch3 at xx30, etc.
* The patchset section needs to begin at an xxf0 address (16 bytes before a page boundary)
**********/

	.MACRO PATCH
patchset:	ld store4, X+		; store4=first patch byte, pattern DDSSPPPP
	ld store2, X+		; store2=second patch byte (patch parameter)
	mov YL, store4		; Use Y as pointer at SRC data
	lsr YL		; Move FF bits to proper position
	andi YL, 0b00011000		; YL is now SRC*8 (each osc output area is 8 bytes)
	subi YL, LOW(-oscdata)	; Add base address, Y now points at output area for SRC
	swap store4		; Move PPPP bits to proper position
	mov ZL, store4		; Z will contain jump address based on patch type
	andi ZL, 0b11110000		; ZL is now patch type * 16 (because each patch type gets 16 words of space)
	eor ZL, twoforty		; Flip the bits in top nibble so 0..15 becomes 15..0
	ldi ZH, HIGH(patch0)	; Low byte for jump set, now set high byte
	andi store4, 0b00001100	; After the jump we'll need Z to point at DST data, get that prepared
	mov store3, store4		; store3 is now DST*4 (each osc scratch area is 4 bytes)
	subi store4, LOW(-scratch)	; Add base address, store4 is now low byte of DST*4
	ijmp		; Jump to desired patch type section, in reverse order
	nop		; Waste one word so setup occupies exactly 16 words

	.IF (PC&0xff)!=0		; Sanity check
	.WARNING "Patch alignment error"	; Make sure the following jump table is starting on a page boundary
	.ENDIF

	; Individual patch sections begin here, in reverse order (e.g. patch0 actually handles patch type 15)
	; Each section starts with setting up Z to point at DST osc scratch area, by copying pointer saved in store4/store5

patch0:	movw ZL, store4		; Type 15=external output pin, parameter=threshhold
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	cp work1, store2		; Compare it to threshhold
	brlo patch0_1		; Branch accodingly
	sbi VPORTD_OUT, 5		; At or above threshhold, set POUT pin high
	rjmp patchnxt
patch0_1:	cbi VPORTD_OUT, 5		; Below threshhold, set POUT pin low
	rjmp patchnxt
	ALIGN 16

patch1:	movw ZL, store4		; Type 14=soft synch multi, parameter=bitmap[--- --- --- REV INV --- --- ---]
	andi store2, 0x18		; Isolate REV & INV bits in parameter byte
	ldd work1, Y+5		; Get SRC cross status byte
	bst work1, 6		; Check status byte for current polarity
	brtc patch1_1		; If in lower half, don't do anything
	ldd work2, Z+3		; Get scratch control byte for DST osc
	eor work2, store2		; Flip the desired bits
	std Z+3, work2		; Store scratch control byte
	ldd work2, Z+7		; Get scratch control byte for DST+1 osc
	eor work2, store2		; Flip the desired bits
	std Z+7, work2		; Store scratch control byte
	ldd work2, Z+11		; Get scratch control byte for DST+2 osc
	eor work2, store2		; Flip the desired bits
	std Z+11, work2		; Store scratch control byte
patch1_1:	rjmp patchnxt
	ALIGN 16

patch2:	movw ZL, store4		; Type 13=hard synch multi, parameter=unused
	ldd work1, Y+5		; Get SRC cross status byte
	bst work1, 1		; Check the overflow indicator
	brtc patch2_1		; If no overflow, don't do anything
	ldd work2, Z+3		; Get scratch control byte for DST osc
	ori work2, 64		; Set RESET control bit
	std Z+3, work2		; Store scratch control byte
	ldd work2, Z+7		; Get scratch control byte for DST+1 osc
	ori work2, 64		; Set RESET control bit
	std Z+7, work2		; Store scratch control byte
	ldd work2, Z+11		; Get scratch control byte for DST+2 osc
	ori work2, 64		; Set RESET control bit
	std Z+11, work2		; Store scratch control  byte
patch2_1:	rjmp patchnxt
	ALIGN 16

patch3:	movw ZL, store4		; Type 12=envelope gate multi, parameter=threshhold
	clt		; Use T-bit as threshhold flag
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	cp work1, store2		; Compare it to threshhold
	brlo patch3_1		; If below threshhold, keep flag clear
	set		; Otherwise set the flag
patch3_1:	ldd work1, Z+3		; Get scratch control byte for DST osc
	bld work1, 7		; Set gate bit from flag
	std Z+3, work1		; Store scratch control byte
	ldd work1, Z+7		; Get scratch control byte for DST+1 osc
	bld work1, 7		; Set gate bit from flag
	std Z+7, work1		; Store scratch control byte
	ldd work1, Z+11		; Get scratch control byte for DST+2 osc
	bld work1, 7		; Set gate bit from flag
	std Z+11, work1		; Store scratch control byte
	rjmp patchnxt
	ALIGN 16

patch4:	movw ZL, store4		; Type 11=amplitude scale multi, parameter=unused
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	ldd work2, Z+16		; Get envelope value for DST osc
	inc work2		; Nudge upward so we can still hit max after scaling
	mul work1, work2		; Multiply to re-scale envelope
	std Z+19, R1		; Store upper byte of scaled value as new envelope output
	ldd work2, Z+20		; Get envelope value for DST+1 osc
	inc work2		; Nudge upward so we can still hit max after scaling
	mul work1, work2		; Multiply to re-scale envelope
	std Z+23, R1		; Store upper byte of scaled value as new envelope output
	ldd work2, Z+24		; Get envelope value for DST+2 osc
	inc work2		; Nudge upward so we can still hit max after scaling
	mul work1, work2		; Multiply to re-scale envelope
	std Z+27, R1		; Store upper byte of scaled value as new envelope output
	rjmp patchnxt
	ALIGN 16

patch5:	movw ZL, store4		; Type 10=amplitude level multi, parameter=scaling factor
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	mul work1, store2		; Adjust using parameter as scaling factor, dropping low byte
	lsr R1		; Reduce from max 254 to max 127
	std Z+19, R1		; Store as new envelope output for DST osc
	std Z+23, R1		; Store as new envelope output for DST+1 osc
	std Z+27, R1		; Store as new envelope output for DST+2 osc
	rjmp patchnxt
	ALIGN 16

patch6:	movw ZL, store4		; Type 9=ring, parameter=unused
	ldd work1, Y+4		; Get SRC osc value (signed)
	mov YL, store3		; We need to re-task Y to point at oscdata, start with the index of DST osc
	lsl YL		; Double the index since oscdata is 8 bytes not 4
	subi YL, -LOW(oscdata+32)	; Now add table offset, voice oscs are in second half of table
	ldd work2, Y+4		; Get DST osc value (signed)
	muls work1, work2		; work1/work2 contain signed values of SRC/DST oscs, so this computes their product
	lsl R0		; Range was cut in half by the signed multiply
	rol R1		; So double result to get back to correct scaling
	sbrc R1, 7		; Need to check one corner case, if -128 x (127/-127/-128) we now overflowed by one bit
	dec R1		; So nudge it back if necessary
	std Y+7, R1		; Store as ring value to be used later
	ldd work1, Z+3		; Get scratch control byte
	ori work1, 32		; Set RING bit to signal presence of ring value
	std Z+3, work1		; Store scratch control byte
	rjmp patchnxt
	ALIGN 16

patch7:	movw ZL, store4		; Type 8=soft sync, parameter=bitmap[--- --- --- REV INV --- --- RES]
	ldd work1, Y+5		; Get SRC cross status byte
	ldd work2, Z+3		; Get DST scratch byte
	lsl work1		; Shift cross status byte to set flags
	brpl patch7_2		; If osc is in lower half, no need to do anything
	brcc patch7_1		; If osc didn't just cross, don't worry about reset
	sbrc store2, 0		; Reset condition met, check if RES bit in parameter is set
	ori work2, 64		; If so, set scratch RESET bit
patch7_1:	andi store2, 0x18		; Isolate REV & INV bits in parameter byte
	eor work2, store2		; Flip indicated bits
	std Z+3, work2		; Store scratch byte
patch7_2:	rjmp patchnxt
	ALIGN 16

patch8:	movw ZL, store4		; Type 7=hard sync, parameter=unused
	ldd work1, Y+5		; Get SRC cross status byte
	bst work1, 1		; Note overflow bit
	brtc patch8_1		; If no overflow, don't do anything
	ldd work2, Z+3		; Get DST scratch control byte
	ori work2, 64		; Set RESET bit
	std Z+3, work2		; Store scratch control byte
patch8_1:	rjmp patchnxt
	ALIGN 16

patch9:	movw ZL, store4		; Type 6=envelope gate, parameter=threshhold
	ldd work1, Z+3		; Get DST scratch control byte
	ldd work2, Y+3		; Get SRC osc value (unsigned)
	cp work2, store2		; Compare to threshhold parameter
	brlo patch9_1		; If below threshhold, turn gate off
	ori work1, 128		; Otherwise turn gate on by setting bit 7
	std Z+3, work1		; Store scratch control byte
	rjmp patchnxt
patch9_1:	andi work1, 127		; Clear bit 7
	std Z+3, work1		; Store scratch control byte
	rjmp patchnxt
	ALIGN 16

patch10:	movw ZL, store4		; Type 5=amplitude level, parameter=signed scaling factor
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	mulsu store2, work1		; Adjust using parameter as scaling factor
	mov work1, R1
	tst work1		; Zero results need special treatment
	brne patch10_1
	sbrc store2, 7		; Check if scaling was negative
	com work1		; When scaling is negative, zero is actually highest value
patch10_1:	andi work1, 127		; Clear bit 7 caused by negative scaling values
	std Z+19, work1		; Store as new envelope output for DST osc
	rjmp patchnxt
	ALIGN 16

patch11:	movw ZL, store4		; Type 4=amplitude scale, parameter=scaling factor
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	mul work1, store2		; Scale it by paramter value (1 byte result)
	ldd work1, Z+16		; Get envelope value for DST osc
	inc work1		; Nudge envelope up so we can reach 100% 
	mul work1, R1		; Multiply to re-scale envelope
	std Z+19, R1		; Store upper byte of scaled value as new envelope output
	rjmp patchnxt
	ALIGN 16

patch12:	movw ZL, store4		; Type 3=midpoint shift, parameter=scaling factor (/256)
	ldd work1, Y+4		; Get SRC osc value (signed)
	mulsu work1, store2		; Scale it by paramter value (1 byte result)
	ldd work1, Z+2		; Get DST scratch midpoint value
	add work1, R1		; Add the calculated shift amount
	std Z+2, work1		; Store new scratch midpoint value
	rjmp patchnxt
	ALIGN 16

patch13:	movw ZL, store4		; Type 2=frequency shift, parameter=scaling factor
	ldd work1, Y+4		; Get SRC osc value (signed)
	mulsu work1, store2		; Scale it by paramter value (signed 2 byte result)
	ldd work1, Z+0		; Get DST scratch frequency low byte
	add work1, R0		; Add the calculated shift amount (low byte)
	std Z+0, work1		; Store the new scratch value
	ldd work1, Z+1		; Get DST scratch frequency high byte
	adc work1, R1		; Add the calculated shift amount (high byte)
	std Z+1, work1		; Store the new scratch value
	rjmp patchnxt
	ALIGN 16

patch14:	movw ZL, store4		; Type 1=frequency scale, parameter=scaling factor
	ldd work1, Y+3		; Get SRC osc value (unsigned)
	mul work1, store2		; Scale it by paramter value (1 byte result)
	inc R1		; Nudge result up so we can reach almost 100% 
	mov store4, R1		; Use result as frequency multiplier
	ldd work1, Z+0		; Get DST scratch frequency low byte
	ldd work2, Z+1		; Get DST scratch frequency high byte
	mul work1, store4		; Multiply 16-bit frequency, dropping low byte
	mov store3, R1
	mul work2, store4
	add R0, store3
	adc R1, zero		; R0/R1 now equivalent of (OSC x SCALE / 256 + 1) x FREQ / 256
	sub work1, R0		; Subtract from original frequency
	sbc work2, R1
	std Z+0, work1		; Store new scratch frequency low byte
	std Z+1, work2		; Store new scratch frequency high byte
	ALIGN 16

patchnxt:			; Type 0 lands here, which simply skips all patch code
	.ENDMACRO
