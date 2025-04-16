/*
 * This example uses the BuzzKill class from the BuzzKill library.
 * It plays a simple 5-note scale with envelope, repeating every 10 seconds.
 * It demonstrates simple envelope settings for Attack and Release of note volume.
 *
 * PLEASE NOTE: This example uses SPI by default. If you have connected your BuzzKill board using I2C instead,
 * see the comments within the setup() function for the appropriate changes.
 *
 * # Released under MIT License
 *
 * Copyright (c) 2025 Todd E. Stidham
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <SPI.h>
#include <Wire.h>
#include <BuzzKill.h>

// Create a BuzzKill object.
BuzzKill buzzkill;

void setup() {
  // If using SPI, the following two lines should be un-commented. If using I2C, the lines should be commented out (or deleted).
  SPI.begin();
  buzzkill.beginSPI();

  // If using I2C, the following two lines should be un-commented. If using SPI, the lines should be commented out (or deleted).
  //Wire.begin();
  //buzzkill.beginI2C();
}

void loop() {
  // Reset all registers to their default values.
  // This is often a good idea when starting a new sequence, to make sure we start from a known state.
  buzzkill.resetRegisters();

  // Configure voice oscillator 0 as a Triangle wave, with starting frequency of 262 Hz.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 0, 262, BUZZKILL_SHAPE_TRIANGLE);

  // Configure an envelope for voice oscillator 0.
  // Use a Linear curve, and Attack and Release times of 700 ms.
  // Sustain level is set at max (127), so we don't need a Decay time (0).
  // Also set the mix volume to max (15), and make sure the note is not playing yet (false).
  buzzkill.configureEnvelope(0, BUZZKILL_CURVE_LINEAR, 700, 0, 127, 700, 15, false);

  // Enable the output for voice oscillator 0. We need this because all voices are disabled by default.
  buzzkill.enableVoice(0);

  // Turn the note on, let it build for 800 ms, then turn it off and let it fade for 700 ms.
  buzzkill.noteOn(0);
  delay(800);
  buzzkill.noteOff(0);
  delay(700);

  // Change the frequency to 294 Hz.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 294);

  // Turn the note on, let it build for 800 ms, then turn it off and let it fade for 700 ms.
  buzzkill.noteOn(0);
  delay(800);
  buzzkill.noteOff(0);
  delay(700);

  // Change the frequency to 330 Hz.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 330);

  // Turn the note on, let it build for 800 ms, then turn it off and let it fade for 700 ms.
  buzzkill.noteOn(0);
  delay(800);
  buzzkill.noteOff(0);
  delay(700);

  // Change the frequency to 392 Hz.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 392);

  // Turn the note on, let it build for 800 ms, then turn it off and let it fade for 700 ms.
  buzzkill.noteOn(0);
  delay(800);
  buzzkill.noteOff(0);
  delay(700);

  // Change the frequency to 440 Hz.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 440);

  // Turn the note on, let it build for 800 ms, then turn it off and let it fade.
  buzzkill.noteOn(0);
  delay(800);
  buzzkill.noteOff(0);

  // Wait 10 seconds before doing it all again.
  delay(10000);
}
