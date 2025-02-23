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
* This code will update the amplitude output of one envelope generator
* The pointer to the "active" generator is stored in gpr_env, and updated each time this code runs
**********/

envelope:	in ZL, gpr_env		; Z now points to envctrl for active envelope
	ldd store1, Z+44		; store1=current level
	ldd store2, Z+45		; store2=current count
	ldd store3, Z+46		; store3=mode/skip count/reload value (CCCRRRMM bit pattern)
	ldd store5, Z+2
	andi store5, 127		; store5=desired sustain level
	ldd work1, Z+31		; scratch area is 28 bytes above envctrl area, so Z+31 is scratch[3]
	bst work1, 7		; Grab gate bit
	mov store4, store3
	andi store4, 3		; store4=mode isolated; 0=off/release, 1=on/attack, 2=on/decay, 3=on/sustain
	brne modenz
	brtc gatez		; Mode is 0 (off), see if gate is also off
	inc store4		; Mode is off but gate is on, switch to attack mode
	rjmp envatt
gatez:	cpi store1, 0		; Gate bit is off, check current level
	brne envskip		; If level>0 keep counting, otherwise nothing to do
envexit:	rjmp envdone
modenz:	brts gatenext		; Mode is >0 (on), see if gate bit is also on
	clr store4		; Mode is on but gate bit is off, switch to release mode
	rjmp envrel
gatenext:	cpi store4, 3		; Mode is on and gate bit is on, check if we're in sustain mode
	breq envsus
envskip:	cpi store3, 32		; Not in sustain mode, check skip count
	brlo envcnt		; If skip count finished, take further action
	subi store3, 32		; Otherwise, update skip count and exit
	rjmp envdone
envcnt:	mov work1, store3		; Skip count is finished
	lsr work1		; We reload the skip count by taking the reload bits and shifting them to the count positions
	swap work1
	andi work1, 224
	or store3, work1		; Skip value now reloaded
	dec store2		; Now update the main counter
	brne envexit		; Exit if it's not done yet
	cpi store4, 1		; Both counts finished, take action depending on what mode we're currently in
	breq envatt
	brsh envdec
	rjmp envrel
envsus:	cp store1, store5		; Sustain mode, just need to compare current level with the desired sustain level
	breq envexit		; If they match, we're done
	sbci store3, 1		; Current level doesn't match, so switch modes depending on which direction we need to go
	rjmp envdone		; All done, we're now either in attack mode(1) or decay mode(2)
envatt:	ldd work3, Z+0		; Attack mode, get control byte
	mov store3, work3		; Save the full control byte value
	andi work3, 3		; Isolate the range value
	ldi work2, 4		; Also need the reverse value (0..3 becomes 3..0)
	sub work2, work3		; Now work2 is our step value
	clr work1		; Figure base timing constant from range value
	sbrc work3, 1
	ldi work1, 8
	sbrc work3, 0
	ldi work1, 6		; work1 is now base delay
	ldd store2, Z+1
	andi store2, 15		; Isolate the attack value
	add store2, work1		; Add the base level, store2 is now total attack value
	brne envatt9		; Check if it's zero
	ldi store1, 127		; If it's zero, we want level to go straight to max
envatt9:	add store1, work2		; Otherwise we'll move toward max by one step
	sbrc store1, 7		; If we went too far...
	ldi store1, 127		; We'll back off to max
	cpi store1, 127		; If we've now reached max...
	breq envatt2		; We'll switch from attack mode to decay mode
	andi store3, 192		; Check envelope curve type
	breq envdec33		; If it's linear just reset counters with same values
	mov XL, store1		; Otherwise we'll figure new values based on current level...
	ldi work1, 127		; Using an exponential lookup table
	sbrc store3, 7		; Based on curve type value...
	eor XL, work1		; We might need to reverse the index
	rjmp envexp
envatt2:	ldi store3, 2		; Attack mode ended, switching to decay mode
	ldi store2, 1		; Set count at minimum so decay mode starts immediately
	rjmp envdone
envdec:	ldd work3, Z+0		; Decay mode, get control byte
	mov store3, work3		; Save the full control byte value
	lsr work3		; Isolate the range value
	lsr work3
	andi work3, 3		; work3=range
	ldi work2, 4		; Also need the reverse value (0..3 becomes 3..0)
	sub work2, work3		; Now work2 is our step value
	clr work1		; Figure base timing constant from range value
	sbrc work3, 1
	ldi work1, 8
	sbrc work3, 0
	ldi work1, 6		; work1 is now base delay
	ldd store2, Z+1
	swap store2
	andi store2, 15		; Isolate the decay value
	add store2, work1		; Add the base level, store2 is now total decay value
	breq envdec2		; If it's zero, we want to go straight to sustain level
	sub store1, work2		; Otherwise we'll move down by one step
	brpl envdec4		; Check if we're still positive
	clr store1		; If not move back to zero
envdec4:	cp store5, store1		; See if we've reached sustain level yet
	brlo envdec3		; Not yet, keep going
envdec2:	mov store1, store5		; We hit it, reload exact value in case we actually overshot it
	ldi store3, 3		; Decay mode ended, switch to sustain mode
	rjmp envdone
envdec3:	andi store3, 192		; Check envelope curve type
envdec33:	breq envrst		; If it's linear just reset counters with same values
	mov XL, store1		; Otherwise we'll figure new values based on current level...
	ldi work1, 127		; Using an exponential lookup table
	sbrs store3, 6		; Based on curve type value...
	eor XL, work1		; We might need to reverse the index
	rjmp envexp
envrel:	ldd work3, Z+0		; Release mode, get control byte
	mov store3, work3		; Save the full control byte value
	swap work3		; Isolate the range value
	andi work3, 3		; work3=range
	ldi work2, 4		; Also need the reverse value (0..3 becomes 3..0)
	sub work2, work3		; Now work2 is our step value
	clr work1		; Figure base timing constant from range value
	sbrc work3, 1
	ldi work1, 8
	sbrc work3, 0
	ldi work1, 6		; work1 is now base release
	ldd store2, Z+3
	andi store2, 15		; Isolate the release value
	add store2, work1		; Add the base level, store2 is now total release value
	breq envrel2		; If it's zero, we want to go straight to min
	sub store1, work2		; Otherwise we'll move down by one step
	brpl envrel3		; Check if we're still positive
envrel2:	clr store1		; If not move back to zero
envrel3:	andi store3, 192		; Check envelope curve type
	breq envrst		; If it's linear just reset counters with same values
	mov XL, store1		; Otherwise we'll figure new values based on current level...
	ldi work1, 127		; Using an exponential lookup table
	sbrs store3, 6		; Based on curve type value...
	eor XL, work1		; We might need to reverse the index
envexp:	ldi XH, HIGH(tabexpon)	; Look up exponential multipliers from table
	ld work1, X		; Fetch low byte
	mul work1, store2		; Multiply by timing value
	mov work2, R1
	ori XL, 128
	ld work1, X		; Fetch high byte
	mul work1, store2		; Multiply by timing value
	mov store2, work2
	add store2, R0		; We now have timing value as exponentional rather than linear
	brne envrst
	inc store2
envrst:	cpi work3, 2		; Calculate the skip count based on the selected range
	brlo envrst2
	inc work3
	ori work3, 3
envrst2:	lsl work3
	mov store3, work3
	swap store3
	lsl work3
	or store3, work3		; Set the skip count and skip reload values in the proper bit locations
	or store3, store4		; Include the mode bits
envdone:	std Z+44, store1		; Store all updated values
	std Z+45, store2
	std Z+46, store3
	std Z+47, store1
	subi ZL, -4		; Advance pointer to next envelope for next frame
	andi ZL, 44		; Cycle through values 32/36/40/44 to point at each envelope in turn
	out gpr_env, ZL		; Save new pointer, all done for this frame
