/*
 * This example uses the BuzzKill class from the BuzzKill library.
 * It plays three separate sound effects, repeating every 10 seconds.
 * It demonstrates using modulation oscillators and patches to alter sound output.
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

  /** Siren Sound Effect **/

  // Reset all registers to their default values.
  // This is often a good idea when starting a new sequence, to make sure we start from a known state.
  buzzkill.resetRegisters();

  // Set the frequency for voice oscillator 0 to 1 KHz.
  // This will provide the base frequency for the siren.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_VOICE, 0, 1000);

  // Set the frequency for modulation oscillator 0 to 1/2 Hz.
  // This will provide the "sweeping" action that alters the base frequency.
  buzzkill.setFrequency(BUZZKILL_OSCTYPE_MOD, 0, 0.5);

  // Add a patch between the two oscillators, so that mod osc 0 alters the frequency of voice osc 0.
  buzzkill.addPatch(0, 0, BUZZKILL_PATCH_FREQSCALE, 40);

  // Enable the output for voice oscillator 0. We need this because all voices are disabled by default.
  buzzkill.enableVoice(0);

  // Start the note playing, and let it play for 3 seconds.
  buzzkill.noteOn(0);
  delay(3000);

  /** Alarm Sound Effect **/

  // Reset all registers to their default values. This will stop all ongoing sounds.
  buzzkill.resetRegisters();

  // Configure voice oscillator 0 as a 500 Hz Pulse wave.
  // This will provide the base frequency for the alarm.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 0, 500, BUZZKILL_SHAPE_PULSE);

  // Configure mod oscillator 0 as an 8 Hz Sine wave.
  // This will provide the "sweeping" action that alters the base frequency.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_MOD, 0, 8, BUZZKILL_SHAPE_SINE);

  // Configure mod oscillator 1 as a 1 Hz Hilltop wave.
  // This will provide the "sweeping" action that alters the amplitude.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_MOD, 1, 1, BUZZKILL_SHAPE_HILLTOP);

  // Add a patch between mod osc 0 and voice osc 0, so that mod osc 0 alters the frequency of voice osc 0.
  buzzkill.addPatch(0, 0, BUZZKILL_PATCH_FREQSCALE, 40);

  // Add a patch between mod osc 1 and voice osc 0, so that mod osc 1 alters the amplitude of voice osc 0.
  buzzkill.addPatch(1, 0, BUZZKILL_PATCH_AMPSCALE, 150);

  // Enable the output for voice oscillator 0.
  buzzkill.enableVoice(0);

  // Start the note playing, and let it play for 3 seconds.
  buzzkill.noteOn(0);
  delay(3000);

  /** Compute Sound Effect **/

  // Reset all registers to their default values. This will stop all ongoing sounds.
  buzzkill.resetRegisters();

  // Configure voice oscillator 0 as a 500 Hz Triangle wave.
  // This will provide the primary frequency.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 0, 500, BUZZKILL_SHAPE_TRIANGLE);

  // Configure voice oscillator 1 as a 770 Hz Pulse wave.
  // This will provide the secondary frequency.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_VOICE, 1, 770, BUZZKILL_SHAPE_PULSE);

  // Set the mix volumes. We want voice 0 to be louder than voice 1.
  buzzkill.setMixVolume(0, 10);
  buzzkill.setMixVolume(1, 4);

  // Configure mod oscillator 0 as an 8 Hz Noise wave.
  // This will provide a randomization factor for the primary frequency.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_MOD, 0, 8, BUZZKILL_SHAPE_NOISE);

  // Configure mod oscillator 1 as an 6 Hz Noise wave.
  // This will provide a randomization factor for the secondary frequency.
  buzzkill.configureOscillator(BUZZKILL_OSCTYPE_MOD, 1, 6, BUZZKILL_SHAPE_NOISE);

  // Add a patch between mod osc 0 and voice osc 0, so that mod osc 0 alters the frequency of voice osc 0.
  buzzkill.addPatch(0, 0, BUZZKILL_PATCH_FREQSHIFT, 20);

  // Add a patch between mod osc 1 and voice osc 1, so that mod osc 1 alters the frequency of voice osc 1.
  buzzkill.addPatch(1, 1, BUZZKILL_PATCH_FREQSHIFT, 50);

 // Enable the output for voices 0 and 1, leave others disabled.
  buzzkill.enableVoice(true, true, false, false);

  // Start voices 0 and 1 playing, let them play for 3 seconds, then turn them off.
  buzzkill.noteOn(true, true, false, false);
  delay(3000);
  buzzkill.noteOn(false, false, false, false);

  // Wait 10 seconds before doing it all again.
  delay(10000);
}
