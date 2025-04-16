/*
 * This example uses the BuzzKill class from the BuzzKill library.
 * It speaks the simple phrase "Hello, my name is BuzzKill" every 10 seconds.
 * It demonstrates the basic functions of speech synthesis.
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

  // Reset all registers to their default values.
  // This is often a good idea when starting a new sequence, to make sure we start from a known state.
  buzzkill.resetRegisters();

  // Clear any phonemes currently in the speech buffer.
  // This is not strictly necessary if you're sure nothing has been added, but it's always good to start clean.
  buzzkill.clearSpeechBuffer();

  // Load the speech buffer with the desired phonemes, using the symbolic tag for each phoneme.
  // See the BuzzKill User's Guide for a complete list of phonemes and their tags.
  buzzkill.addSpeechTags("H* EH L* OW _3 M* AY N* EY M* IH Z* _1 B* AH Z* K* IH L*");

  // Set oscillators, envelopes and patches to proper values for producing speech at a pitch of 160 Hz.
  buzzkill.prepareSpeechMode(160, BUZZKILL_PATCH_HARDSYNCMULTI);
}

void loop() {
  // Begin speaking from the speech buffer.
  // Note that since we are using the same phrase each time, we don't need to re-do any of the set up.
  buzzkill.startSpeaking();

  // Wait 10 seconds before doing it all again.
  delay(10000);
}
