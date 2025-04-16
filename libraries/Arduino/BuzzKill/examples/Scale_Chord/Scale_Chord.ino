/*
 * This example uses the BuzzKill class from the BuzzKill library.
 * It plays a 5-note scale of chords, repeating every 10 seconds.
 * It demonstrates using multiple voices at once.
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

  // Configure voice oscillator 0 as a Sine wave, with starting frequency of 262 Hz.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 0, 262, BUZZKILL_SHAPE_SINE);

  // Configure voice oscillator 1 as a Triangle wave, with starting frequency of 349 Hz.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 1, 349, BUZZKILL_SHAPE_TRIANGLE);

  // Configure voice oscillator 2 as a Ramp wave, with starting frequency of 393 Hz.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 2, 393, BUZZKILL_SHAPE_RAMP);

  // Set the mix volume for each oscillator to about 1/3 of max, since there are three voices.
  buzzkill.setMixVolume(0, 4);
  buzzkill.setMixVolume(1, 4);
  buzzkill.setMixVolume(2, 4);

  // Enable the output for our three oscillators, leave the fourth one disabled.
  buzzkill.enableVoice(true, true, true, false);

  // Start the three notes playing, and let them play for 1 second.
  buzzkill.noteOn(true, true, true, false);
  delay(1000);

  // Change the frequency for each oscillator, and let them play for 1 second.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 294);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 1, 392);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 2, 440);
  delay(1000);

  // Change the frequency for each oscillator, and let them play for 1 second.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 330);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 1, 440);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 2, 495);
  delay(1000);

  // Change the frequency for each oscillator, and let them play for 1 second.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 392);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 1, 523);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 2, 588);
  delay(1000);

  // Change the frequency for each oscillator, and let them play for 1 second.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 440);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 1, 587);
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 2, 660);
  delay(1000);

  // Stop the notes and wait 10 seconds before doing it all again.
  buzzkill.noteOn(false, false, false, false);
  delay(10000);
}
