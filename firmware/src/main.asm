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

; Un-comment the following line only if you are assembling in a stand-alone environment
; If you are using Microchip Studio, this file is added automatically and doesn't need to be expicitly included here
;	.INCLUDE "AVR16DD14def.inc"

; Define a macro for aligning code to a desired boundary. We use this a lot, so we'll define it here at top level.
	.MACRO ALIGN		; Usage: ALIGN 16, ALIGN 32, ALIGN 256, etc.
	.IF PC&(@0-1)		; Check if we're already at the specified alignment
	.ORG (PC&(0x10000-@0))+@0	; If not, skip ahead enough words to reach the next aligned address
	.ENDIF
	.ENDMACRO

	.INCLUDE "buildopts.inc"
	.INCLUDE "registers.inc"
	.INCLUDE "patch.inc"
	.INCLUDE "oscillator.inc"
	.INCLUDE "interrupt.inc"
	.INCLUDE "tables.inc"

.DSEG
.ORG SRAM_START
	.INCLUDE "rammap.inc"

.CSEG

; The following code sections are placed at fixed addresses, either by hardware requirement (vector tables) or for efficiency in handling interrupts (jump tables)

.ORG 0x0000			; Reset/Power-up vector
	jmp reset

.ORG 0x0012			; TCA0_OVF_vect, called when TimerA0 overflows approx. 16384 times per second
	andi work2, 0xfe		; work2 will contain 0x88 or 0x89, clear low bit and set N status flag
	sts TCA0_SINGLE_CTRLESET, work2	; Restart TimerA0 and clear interrupt flag
	reti
.ORG 0x0024			; TWI0_TWIS_vect, called when TWI attention is needed
	in savestat, CPU_SREG	; We don't have room here, so just save status and jump to real code
	rjmp twi

.ORG 0x0028			; SPI0_INT_vect, called when SPI attention is needed
	in savestat, CPU_SREG	; Save status
	movw saveptr, ZL		; Save Z register
	lds intreg, SPI0_INTFLAGS	; Reading the INTFLAGS register is necessary to clear the interrupt flag
	in ZL, gpr_mode		; Point to specific handler based on current mode
	ldi ZH, 1		; All handler blocks for SPI are on code page 1
	ijmp		; Jump to handler, any needed restores/returns will be handled there

.ORG 0x0044			; PORTF_PORT_vect, called when SPI SS goes from high to low
	sbi VPORTF_INTFLAGS, 7	; We only activate this in sleep mode so we know when to wake up
	reti

.ORG 0x00a0			; A convenient place to put the remaining TWI interrupt code we couldn't fit in the vector slot
twi:	lds intreg, TWI0_SSTATUS	; Need to check if this is an address byte or a data byte
	lsl intreg
	brmi twiaddr		; Address byte, jump to address received code
	movw saveptr, ZL		; Save Z register
	in ZL, gpr_mode		; Point to specific handler based on current mode
	ldi ZH, 3		; All handler blocks for TWI are on code page 3
	ijmp		; Jump to handler, any needed restores/returns will be handled there
twiaddr:	inc intreg		; Load the proper address acknowledge command
	sts TWI0_SCTRLB, intreg	; Acknowledge the address
	out CPU_SREG, savestat	; Restore status
	reti

.ORG 0x00b0			; Actual SPI interrupt code, we start this near the end of page 0 so all the associated handler blocks will fit on page 1
	INTERRUPT SPI0_DATA		; Insert code, specifying that it reads from SPI0_DATA register
	.IF handlerblock!=1		; Do a sanity check, make sure our handler block is on page 1 as expected
	.WARNING "Int handler wrong page"	; If we start too close to the end of the page, code would wrap to next page and handler block would be pushed a further page ahead
	.ENDIF		; We could handle that automatically, but it's also an indicator of wasted space so we treat it as an error

.ORG 0x0200			; Since we're hand-aligning the interrupt handlers to use mostly page 1 and 3, we have a bit of room here we can fill in

; Here are some universal routines that may be called by either SPI or TWI interrupt handlers, as well as non-interrupt code
; We put them here to be close to the interrupt code and init code which refers to them, but they could really go anywhere
; Note that these exist at top level, so an interrupt routine must handle restoring registers and RETI before branching here

gosleep:	lds work1, SPI0_CTRLA	; Check which protocol we're using
	bst work1, SPI_ENABLE_bp	; If set, we're using SPI
	ldi work1, 0b10110000	; Change PD4, PD5, PD7 to input (floating)
	sts PORTD_DIRCLR, work1	; PD4=SOUT, PD5=POUT, PD7=AMPEN
	brtc gosleep2		; If using TWI, nothing extra needed
	sbi VPORTD_INTFLAGS, 7	; Clear any pending interrupt on SS pin
	ldi work2, PORT_ISC_FALLING_gc	; Configure SS pin with input buffer on and interrupt on falling edge
	sts PORTF_PIN7CTRL, work2	; PF7=SS pin for SPI connection
gosleep2:	sleep
	brtc gosleep3		; If using TWI, nothing extra needed
	ldi work2, PORT_ISC_INPUT_DISABLE_gc	; Reconfigure SS pin with input buffer off and no interrupt
	sts PORTF_PIN7CTRL, work2	; PF7=SS pin for SPI connection
gosleep3:	sts PORTD_DIRSET, work1	; Change PD4, PD5, PD7 back to outputs
	jmp wait

newtwiaddr:	ldi XL, LOW(twiaddrbuff)	; If three bytes in buffer are valid, set new address and store in eeprom
	ldi XH, HIGH(twiaddrbuff)
	ld store1, X+
	ldi work1, 0b01010101
	eor work1, store1
	ld store2, X+
	cp work1, store2
	brne newtwi2
	ldi work1, 0b10101010
	eor work1, store1
	ld store3, X
	cp work1, store3
	brne newtwi2
	ldi XL, LOW(TWI_EEPROM)
	ldi XH, HIGH(TWI_EEPROM)
	ldi work3, CPU_CCP_SPM_gc
newtwi1:	lds work1, NVMCTRL_STATUS
	andi work1, NVMCTRL_EEBUSY_bm
	brne newtwi1
	out CPU_CCP, work3
	ldi work1, NVMCTRL_CMD_EEERWR_gc
	sts NVMCTRL_CTRLA, work1
	st X+, store1
	st X+, store2
	st X, store3
	out CPU_CCP, work3
	ldi work1, NVMCTRL_CMD_NOOP_gc
	sts NVMCTRL_CTRLA, work1
	lsl store1
	sts TWI0_SADDR, store1
newtwi2:	jmp wait

; This is just a relay jump, an absolute jump to a location that is too far away to use relative addressing
; Instructions that need to use relative addressing can branch/jump here instead and then will jump to the final destination
relay_speech:	jmp speech

.ORG 0x02b0			; Actual TWI interrupt code, we start this near the end of page 2 so all the associated handler blocks will fit on page 3
	INTERRUPT TWI0_SDATA	; Insert code, specifying that it reads from TWI0_SDATA register
	.IF handlerblock!=3		; Do a sanity check, make sure our handler block is on page 3 as expected
	.WARNING "Int handler wrong page"	; If we start too close to the end of the page, code would wrap to next page and handler block would be pushed a further page ahead
	.ENDIF		; We could handle that automatically, but it's also an indicator of wasted space so we treat it as an error

; This is a semi-arbitrary starting point for our main code (see note under patches section for why this may need to be adjusted if code changes)
.ORG 0x0436

; Here the main loop begins. We should end up back here approximately every 61 usec
main:	ldi YL, 0x09		; Switch interrupts from timer mode to input mode
	ldi YH, 0x0a		; Y now points to 0x0a09 (one below TCA0_SINGLE_INTCTRL)
	std Y+1, zero		; TCAO overflow interrupt now inactive
	dec YH		; Y now points to 0x0909 (TWI0_SCTRLA)
	ld work1, Y		; Load current SCTRLA value
	ori work1, TWI_DIEN_bm|TWI_APIEN_bm	; Set data and address interrupt bits
	st Y, work1		; TWI interrupts now active (if TWI peripheral is enabled)
	std Y+57, YL		; Set interrupt bit in SPI0_INTCTRL, SPI interrupt now active (if SPI peripheral is enabled)

	sbic gpr_spch, 7		; Check if we're in speech mode
	rjmp relay_speech		; If so, jump there and skip all this

	.INCLUDE "envelope.inc"	; Update the envelope for one voice (rotate active envelope each time through)

patches:	ldi YL, LOW(oscctrl+16)
	ldi YH, HIGH(ctrlgrid)
	ldi ZL, LOW(scratch)
patchinit:	ld work1, Y+
	st Z+, work1
	ld work1, Y+
	st Z+, work1
	ld work1, Y+
	st Z+, work1
	ld work1, Y+
	andi work1, 0x1f
	ldd work2, Y+14		; copy GAT bit from matching envctrl[2]
	andi work2, 128
	or work1, work2
	st Z+, work1
	cpi YL, LOW(envctrl)
	brlo patchinit
	ldi XL, LOW(ptchctrl)
	ldi XH, HIGH(ptchctrl)
	ldi store1, LOW(ptchctrl+10)
	ldi store5, HIGH(ctrlgrid)	; store5=high byte of registers (normal ZH value)

sanity:	; Good place for a sanity check! We need to make sure we're exactly 16 words from the start of a new page
	; If we're off we can just adjust the ORG of the main loop as needed. Each patch fills exactly one page, so aligning the first one aligns all of them
	.IF (PC&0xff)!=0xf0
	.WARNING "Patch alignment error"	; Need to start 16 words back so jump tables line up with page boundaries
	.ENDIF

	PATCH 0		; Insert the patch code five times
	PATCH 1
	PATCH 2
	PATCH 3
	PATCH 4

	bst shifth, 6		; Update the noise shift register
	bld work1, 0
	eor work1, shiftl
	lsr work1
	rol shiftl
	rol shifth
	clr outputl		; Clear output registers
	clr outputh
	; Set up the registers so we can iterate through the oscillators
	ldi ZH, HIGH(ctrlgrid)	; This was clobbered by patch code, need to reset it
	ldi ZL, LOW(oscctrl)	; Point Z at osc control
	ldi YL, LOW(oscdata)	; Point Y at osc data (YH already set correctly)
	lds work3, rstmask
	sts rstmask, zero		; Clear resetmask bits
	ldd aux1, Z+49		; RST bits, rotate and check for each oscillator
	or aux1, work3		; Any resetmask bits will have same effect as RST bits
	ldd work3, Z+48		; Voice enables and volume, rotate and check for each osc #4-7

	; Each oscillator is handled in two code blocks called "A" and "B"
	; There is a gap between each block caused by the need for page alignment, so we can use the otherwise wasted space to store a data table
	OSCILLATORA 0		; Do first part of code for Oscillator 0
	TABLE_FABMIDL		; Stuff a data table here to fill some empty space
	OSCILLATORB 0		; Now do the second part of the oscillator code
	.IF oscpagetarg!=oscpagedest	; Do a sanity check, make sure our calculated jump page was accurate
	.WARNING "Osc0 jump target mismatch"	; If the table we stuffed here was too long, code would wrap to next page
	.ENDIF		; Jumps would then point to wrong code, so we check for it here
	OSCILLATORA 1		; Repeat the pattern for the other oscillators and other data tables
	TABLE_FABMIDH
	OSCILLATORB 1
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc1 jump target mismatch"
	.ENDIF
	OSCILLATORA 2
	TABLE_FABEXPON
	OSCILLATORB 2
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc2 jump target mismatch"
	.ENDIF
	OSCILLATORA 3
	TABLE_FABDFLTS
	OSCILLATORB 3
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc3 jump target mismatch"
	.ENDIF
	OSCILLATORA 4		; Osc 4-7 will add to outputl/outputh based on enable and volume settings
	TABLE_FABSPCHDEF
	OSCILLATORB 4
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc4 jump target mismatch"
	.ENDIF
	OSCILLATORA 5
	TABLE_FABSTEP
	OSCILLATORB 5
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc5 jump target mismatch"
	.ENDIF
	OSCILLATORA 6
	OSCILLATORB 6
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc6 jump target mismatch"
	.ENDIF
	OSCILLATORA 7
	OSCILLATORB 7
	.IF oscpagetarg!=oscpagedest	; Do our sanity check
	.WARNING "Osc7 jump target mismatch"
	.ENDIF
	; After this outputl/outputh contains the total oscillator output

; Done with all oscillator processing, now we wait to make sure we've completed the number of cycles necessary for this frame
wait:	cli
	lds work1, TCA0_SINGLE_CNTL
	lds work2, TCA0_SINGLE_CNTH
	cpi work2, 5		; Check if we have lots of time to kill
	brlo waitinp		; If so, kill chunk of time with input mode active
	ldi YL, 0x09		; Switch interrupts from input mode to timer mode
	ldi YH, 0x09		; Y now points to 0x0909 (TWI0_SCTRLA)
	ld work2, Y		; Load current SCTRLA value
	andi work2, ~(TWI_DIEN_bm|TWI_APIEN_bm)	; Clear data and address interrupt bits
	st Y, work2		; TWI interrupts now inactive
	std Y+57, zero		; Clear interrupt bit in SPI0_INTCTRL, SPI interrupt now inactive
	inc YH		; Y now points to 0x0a09 (one below TCA0_SINGLE_INTCTRL)
	std Y+1, YL		; TCAO overflow interrupt now active
	inc framecnt
	mov XL, framecnt
	ldi XH, HIGH(tabsynch)
	ld work2, X
	eor work1, work2
	lsr work1
	sei		; Now we need to catch timer interrupt
	brcc wait2
wait2:	brpl wait2		; Wait here until N status flag set, which happens when TimerA0 overflows

; Cycle count now correct, just need to send our final output to the DAC
output:	inc work3		; Main volume now 1-16 (was previously shifted right 4 times by oscillator code)
	mul outputl, work3		; Multiply osc output by main volume to get final output value
	mov store2, R1
	mulsu outputh, work3
	mov store3, R1
	add store2, R0
	adc store3, zero		; Volumized output now 11-bit signed value in store2/store3
	subi store3, -4		; Convert to 10-bit unsigned suitable for DAC
	lsr store3
	ror store2
	lsr store3
	ror store2
	ror store3
	ror store2
	ror store3		; 10-bit unsigned, bits 0-1 at top of store3 and bits 2-9 in store2
	sts DAC0_DATAL, store3	; Output DAC low byte
	sts DAC0_DATAH, store2	; Output DAC high byte
	jmp main		; Do it all again

waitinp:	sei		; Coarse time-kill with input interrupts active
	com work1		; Wait here as long as possible so any new inputs can be processed
	lsr work1
	lsr work1
waitinp2:	dec work1
	brpl waitinp2
	rjmp wait		; Once we're close to end of frame, jump back to waiting in timer mode

	.INCLUDE "speech.inc"

; We have some room here for data tables we haven't already placed
	TABLE_SINE
	TABLE_PHONLEN
	TABLE_FORMANTS


; Here is the main reset/initialization code, called on power-up
reset:	ldi work1, CPU_CCP_IOREG_gc	; First we enable system clock using the external crystal
	sts CPU_CCP, work1
	ldi work1, CLKCTRL_RUNSTBY_bm | CLKCTRL_CSUTHF_4K_gc | CLKCTRL_FRQRANGE_24M_gc | CLKCTRL_SELHF_XTAL_gc | CLKCTRL_ENABLE_bm
	sts CLKCTRL_XOSCHFCTRLA, work1
clk1:	lds work1, CLKCTRL_MCLKSTATUS
	andi work1, CLKCTRL_EXTS_bm
	breq clk1
	ldi work1, CPU_CCP_IOREG_gc
	sts CPU_CCP, work1
	ldi work1, CLKCTRL_CLKSEL_EXTCLK_gc	; Switch to the new clock
	sts CLKCTRL_MCLKCTRLA, work1
clk2:	lds work1, CLKCTRL_MCLKSTATUS	; Wait for main 24MHz clock to start and stabilize
	andi work1, CLKCTRL_SOSC_bm
	brne clk2

	clr zero		; Set up fixed-value registers
	ldi work1, 240
	mov twoforty, work1

	ldi work1, VREF_REFSEL_VDD_gc	; Enable DAC with VDD set as reference voltage
	sts VREF_DAC0REF, work1
	ldi work1, DAC_OUTEN_bm | DAC_ENABLE_bm
	sts DAC0_CTRLA, work1

	ldi work1, LOW(1455)	; Enable TimerA0 with period of 1455 cycles
	sts TCA0_SINGLE_PERL, work1	; With overhead this is approximately 16384 Hz
	ldi work1, HIGH(1455)	; Extra cycles will be added as needed to fine-tune frequency
	sts TCA0_SINGLE_PERH, work1
	sts TCA0_SINGLE_INTCTRL, zero
	ldi work1, TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm
	sts TCA0_SINGLE_CTRLA, work1

	out VPORTA_DIR, zero
	out VPORTA_OUT, zero
	out VPORTC_DIR, zero
	out VPORTC_OUT, zero
	out VPORTD_DIR, zero
	out VPORTD_OUT, zero
	out VPORTF_DIR, zero
	out VPORTF_OUT, zero
	ldi work1, PORT_ISC_INPUT_DISABLE_gc	; Default config for all pins (input buffer off, no pullup, no interrupt)
	sts PORTA_PINCONFIG, work1	; We will explicitly override this as needed for our software-controlled pins
	sts PORTC_PINCONFIG, work1	; Some others will be overridden by hardware control when SPI/TWI modules are enabled
	sts PORTD_PINCONFIG, work1
	sts PORTF_PINCONFIG, work1
	ldi work1, PORT_ISC_INTDISABLE_gc|PORT_PULLUPEN_bm	; Temporarily set PROTO pin to digital input to check SPI/TWI option
	sts PORTF_PIN6CTRL, work1	; PF6=PROTO (protocol/interface select)
	ldi work1, 0b10110000	; Designate PD4, PD5, PD7 as outputs
	sts PORTD_DIRSET, work1	; PD4=SOUT, PD5=POUT, PD7=AMPEN
	ldi work1, 0b10000000	; Set PD7 high (enable amplifier), others low
	sts PORTD_OUTSET, work1
	ldi work1, SLPCTRL_SMODE_PDOWN_gc|SLPCTRL_SEN_bm	; Configure sleep command for power-down mode
	sts SLPCTRL_CTRLA, work1

	; Decide whether to initialize SPI or TWI module, based on build options and reading from PROTO pin
	.IF PROTOCOL_SEL==0
	sbis VPORTF_IN, 6
	.ELIF PROTOCOL_SEL==1
	sbic VPORTF_IN, 6
	.ENDIF
	.IF PROTOCOL_SEL!=2
	rjmp inittwi
	.ENDIF
initspi:	ldi work1, 6		; Assign SPI pinout -- MOSI(PC1/6), SCK(PC3/8), SS(PF7/3)
	sts PORTMUX_SPIROUTEA, work1
	ldi work1, SPI_ENABLE_bm
	sts SPI0_CTRLA, work1
	rjmp protdone
inittwi:	ldi XL, LOW(TWI_EEPROM)
	ldi XH, HIGH(TWI_EEPROM)
	ld store1, X+
	ld store2, X+
	ldi work1, 0b01010101
	eor work1, store1
	cp work1, store2
	brne inittwi2
	ld store3, X
	ldi work1, 0b10101010
	eor work1, store1
	cpse work1, store3
inittwi2:	ldi store1, TWI_ADDR_DEF
	lsl store1
	sts TWI0_SADDR, store1
	ldi work1, 2		; Assign TWI pinout -- SDA(PC2/7), SCL(PC3/8)
	sts PORTMUX_TWIROUTEA, work1
	ldi work1, TWI_FMPEN_bm
	sts TWI0_CTRLA, work1
	ldi work1, TWI_SMEN_bm|TWI_ENABLE_bm
	sts TWI0_SCTRLA, work1
protdone:	ldi work1, PORT_ISC_INPUT_DISABLE_gc	; We don't need to read PROTO pin again so reset its config
	sts PORTF_PIN6CTRL, work1

inittabs:	ldi ZL, LOW(fabmidl<<1)	; Initialize various RAM tables by copying from FLASH
	ldi ZH, HIGH(fabmidl<<1)	; This allows faster access later (read from RAM is faster than read from FLASH)
	ldi YL, LOW(tabmidl)	; Initialize tabmidl (midpoint table low bytes)
	ldi YH, HIGH(tabmidl)
cpmidl:	lpm work1, Z+
	st Y+, work1
	tst YL
	brne cpmidl
	ldi ZL, LOW(fabmidh<<1)	; Initialize tabmidh (midpoint table high bytes)
	ldi ZH, HIGH(fabmidh<<1)
	ldi YL, LOW(tabmidh)
	ldi YH, HIGH(tabmidh)
cpmidh:	lpm work1, Z+
	st Y+, work1
	tst YL
	brne cpmidh
	ldi ZL, LOW(fabexpon<<1)	; Initialize tabexpon (exponential table)
	ldi ZH, HIGH(fabexpon<<1)
	ldi YL, LOW(tabexpon)
	ldi YH, HIGH(tabexpon)
cpexpon:	lpm work1, Z+
	st Y+, work1
	tst YL
	brne cpexpon
	ldi ZL, LOW(fabstep<<1)	; Initialize tabstep (waveform step table)
	ldi ZH, HIGH(fabstep<<1)
	ldi YL, LOW(tabstep)
	ldi YH, HIGH(tabstep)
cpstep:	lpm work1, Z+
	st Y+, work1
	cpi YL, 8
	brlo cpstep
	ldi ZL, LOW(fabdflts<<1)	; Initialize tabdflts (register default values table)
	ldi ZH, HIGH(fabdflts<<1)
	ldi YL, LOW(tabdflts)
	ldi YH, HIGH(tabdflts)
cpdef:	lpm work1, Z+
	std Y+60, work1		; In addition to storing value in default values table, also store it 60 bytes ahead
	st Y+, work1		; That way we fill in ctrlgrid workspace with default values at same time we fill in table
	tst YL
	brne cpdef
	ldi ZL, LOW(fabspchdef<<1)	; Need to initialize spchctrl workspace as well
	ldi ZH, HIGH(fabspchdef<<1)	; Never need to do this again, so no need to build storage table
	ldi YL, LOW(spchctrl)	; Just copy values straight from FLASH to workspace
	ldi YH, HIGH(spchctrl)
	ldi work2, 37
cpspc:	lpm work1, Z+
	st Y+, work1
	dec work2
	brne cpspc
	clr XL		; Build synch table, which determines whether each frame is 1464 or 1465 cycles long
	ldi XH, HIGH(tabsynch)	; We want 16384 Hz frame rate, with 24 MHz clock that works out to 1464.84375 cycles per frame
sync1:	ldi work2, 0x88		; We alternate frame length in a fixed pattern so that average frame length is 1464.84375 frames
	mov work1, XL		; This table will be referenced at the end of each frame to determine the proper length (1464/1465)
	andi work1, 0x1f		; Values in table are either 0x88 or 0x89, only the low bit of each value has relevance for frame length
	cpi work1, 0x1c		; The other bits merely increase economy in the later code by pre-loading a certain flag pattern
	breq sync2
	andi work1, 0x07
	breq sync2
	ori work2, 1
sync2:	st X, work2
	inc XL
	brne sync1
	ldi YL, LOW(spchbuff)	; Clear speech buffer
	ldi YH, HIGH(spchbuff)
sbclear:	st Y, zero
	inc YL
	brne sbclear
	ldi work1, 0xfe		; Initialize the noise shift register
	mov shiftl, work1
	inc work1
	mov shifth, work1
	out gpr_mode, zero		; Set fixed-purpose registers to starting values
	out gpr_reg, zero
	out gpr_spch, zero
	mov datacnt, zero
	mov sblast, zero
	ldi work1, LOW(envctrl)
	out gpr_env, work1
	ldi ZH, HIGH(ctrlgrid)	; For efficiency we pre-set ZH, if we need to clobber it we always set it back

	; If build options specify automatic sleep mode, insert that here
	.IF SLEEP_AT_BOOT==1
	jmp gosleep
	.ELSE
	jmp main
	.ENDIF
