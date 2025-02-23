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
* Here we define all the registers with easier symbolic names
* Most of them are general-purpose and used for many a variety of tasks, so the names are still very non-specific
* A few are used for a specific purpose throughout the program and have names reflecting that usage
* We don't define R0 or R1 so we keep those free for multiplication results
* We don't define R26-R31 so we keep them free for use as X, Y, and Z pointers
**********/

/**********
* First are the ordinary registers, R2-R15
* These are less useful for calculations and bit manipulations, so we use them mainly for longer-term storage and fixed values
**********/

.DEF	zero	= r2	; Fixed value (0) loaded during init and maintained throughout program
.DEF	twoforty	= r3	; Fixed value (240) loaded during init and maintained throughout program
.DEF	shiftl	= r4	; Low byte of noise generator shift register
.DEF	shifth	= r5	; High byte of noise generator shift register
.DEF	outputl	= r6	; Low byte of output accumulator (total output of all voices), note that high byte is not contiguous
.DEF	framecnt	= r7	; Frame counter, updated after each output and used to keep cycle count synchronized
.DEF	intreg	= r8	; Dedicated ISR register, guaranteed to cause no harm if clobbered
.DEF	savestat	= r9	; Dedicated ISR register, used to save STATUS byte during interrupt
.DEF	saveptr	= r10	; Dedicated ISR register, used to save low byte of X/Y/Z pointer if needed by interrupt code
.DEF	saveptr2	= r11	; Dedicated ISR register, used to save high byte of X/Y/Z pointer if needed by interrupt code
.DEF	sblast	= r12	; Speech buffer pointer, stores index of last speech phoneme loaded
.DEF	aux1	= r13	; General long-term usage
.DEF	aux2	= r14	; General long-term usage
.DEF	aux3	= r15	; General long-term usage

/**********
* Next are the "priviledged" registers, R16-R25
* These are mostly for short-term use in data transfers and calculations, or any instruction that needs a high register
**********/

.DEF	work1	= r16	; General short-term usage
.DEF	work2	= r17	; General short-term usage
.DEF	work3	= r18	; General short-term usage
.DEF	datacnt	= r19	; Data count, used mostly to keep track of byte counts for SPI/TWI command sequences
.DEF	outputh	= r20	; High byte of output accumulator (total output of all voices), note that low byte is not contiguous
.DEF	store1	= r21	; General medium-term usage
.DEF	store2	= r22	; General medium-term usage
.DEF	store3	= r23	; General medium-term usage
.DEF	store4	= r24	; General medium-term usage
.DEF	store5	= r25	; General medium-term usage

/**********
* Last are the GPRs, special I/O locations that aren't really registers but provide fast access and bit manipulation
* We use them for status info on recurring tasks
**********/

.EQU	gpr_env	= GPR_GPR0	; Pointer to currently active envelope, rotates each frame
.EQU	gpr_mode	= GPR_GPR1	; Command mode, keeps track of current status of SPI/TWI sequence
.EQU	gpr_reg	= GPR_GPR2	; Register index, keeps track of destination register during SPI/TWI data sequence
.EQU	gpr_spch	= GPR_GPR3	; Speech mode, holds status bits used during speech sequences
