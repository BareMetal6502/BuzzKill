/*
 * This file is part of the Arduino library for the BuzzKill Sound Effects Board
 *
 * Version 1.0.0, last updated April 15, 2025
 *
 * Copyright (c) 2025 Todd E. Stidham
 *
 * MIT license, all text here must be included in any redistribution
 */

#ifndef BUZZKILL_H
#define BUZZKILL_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#define BUZZKILL_SPI_SPEED 400000

enum buzzkill_osctype_t: byte {
    BUZZKILL_OSCTYPE_MOD = 0x00,
    BUZZKILL_OSCTYPE_VOICE = 0x10
};

enum buzzkill_shape_t: byte {
    BUZZKILL_SHAPE_SINE = 0x00,
    BUZZKILL_SHAPE_RAMP = 0x20,
    BUZZKILL_SHAPE_TRIANGLE = 0x40,
    BUZZKILL_SHAPE_PULSE = 0x60,
    BUZZKILL_SHAPE_EXPON = 0x80,
    BUZZKILL_SHAPE_NOISE = 0xa0,
    BUZZKILL_SHAPE_CUSTOM = 0xc0,
    BUZZKILL_SHAPE_HILLTOP = 0xe0
};

enum buzzkill_curve_t: byte {
    BUZZKILL_CURVE_LINEAR = 0x00,
    BUZZKILL_CURVE_INVATT = 0x40,
    BUZZKILL_CURVE_INVDEC = 0x80,
    BUZZKILL_CURVE_NATURAL = 0xc0
};

enum buzzkill_patch_t: byte {
    BUZZKILL_PATCH_NONE = 0x00,
    BUZZKILL_PATCH_FREQSCALE = 0x01,
    BUZZKILL_PATCH_FREQSHIFT = 0x02,
    BUZZKILL_PATCH_MIDSHIFT = 0x03,
    BUZZKILL_PATCH_AMPSCALE = 0x04,
    BUZZKILL_PATCH_AMPLEVEL = 0x05,
    BUZZKILL_PATCH_ENVGATE = 0x06,
    BUZZKILL_PATCH_HARDSYNC = 0x07,
    BUZZKILL_PATCH_SOFTSYNC = 0x08,
    BUZZKILL_PATCH_RINGMOD = 0x09,
    BUZZKILL_PATCH_AMPSCALEMULTI = 0x0a,
    BUZZKILL_PATCH_AMPLEVELMULTI = 0x0b,
    BUZZKILL_PATCH_ENVGATEMULTI = 0x0c,
    BUZZKILL_PATCH_HARDSYNCMULTI = 0x0d,
    BUZZKILL_PATCH_SOFTSYNCMULTI = 0x0e,
    BUZZKILL_PATCH_OUTPUTPIN = 0x0f
};

class BuzzKill {
public:
    /**
     * Constructor, no parameters.
     */
    BuzzKill();


    /**
     * Initialize communication in SPI mode. SPI.begin() should be called before calling this function.
     * @param pinSS          (optional) The pin to use for SS functionality; defaults to global SS definition
     * @param spi            (optional) The SPI object to use; usually no need to specify this
     */
    void beginSPI(byte pinSS=SS,
                  SPIClass &spi=SPI);


    /**
     * Initialize communication in I2C mode (TWI). Wire.begin() should be called before calling this function.
     * You should also consider calling Wire.setClock(400000) unless your hardware setup requires lower speeds.
     * @param address        (optional) The 7-bit I2C address the board is using. Defaults to the standard 0x0a address
     * @param wire           (optional) The Wire object to use; usually no need to specify this
     */
    void beginI2C(byte address=10,
                  TwoWire &wire=Wire);


    /**
     * Set the value of one or more board registers.
     * The first value is written to the specified start register, any additional values are written to consecutive registers.
     * @param regStart       The starting register number (0..59)
     * @param val1           The value to be written to the starting register
     * @param val2...        (optional) Additional values to be written to consecutive registers
     */
    void setRegister(byte regStart,
                     byte val1,
                     int16_t val2=-1, int16_t val3=-1, int16_t val4=-1, int16_t val5=-1, int16_t val6=-1, int16_t val7=-1, int16_t val8=-1, int16_t val9=-1, int16_t val10=-1);


    /**
     * Pull values from a byte array and write them to board registers.
     * The first value is written to the specified start register, any additional values are written to consecutive registers.
     * @param regStart       The starting register number (0..59)
     * @param regData        The array of byte values to pull from
     * @param length         Total number of values to pull and write
     */
    void writeRegisters(byte regStart,
                        byte regData[],
                        byte length);


    /**
     * Pull values from a char array or string literal and write them to board registers.
     * The first value is written to the specified start register, any additional values are written to consecutive registers.
     * @param regStart       The starting register number (0..59)
     * @param regData        The array of char values (or string) to pull from
     * @param length         Number of values to pull and write
     */
    void writeRegisters(byte regStart,
                        char regData[],
                        byte length);


    /**
     * Reset registers to default values, optionally starting at a specified register.
     * This will disable all sound output and clear all modulation patches.
     * It does not clear the speech buffer or affect speech speed or speech factors.
     * @param regStart       (optional) The starting register number (0..59); defaults to 0
     */
    void resetRegisters(byte regStart=0);


    /**
     * Configure all options for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param freqency       The oscillator frequency (0.0 - 4095.9375); will be set to next lowest 1/16th
     * @param shape          The waveform shape (BUZZKILL_SHAPE_SINE, BUZZKILL_SHAPE_RAMP, etc.)
     * @param midpoint       (optional) The oscillator midpoint value, default 128
     * @param invert         (optional) Whether to invert the waveform (true/false), default false
     * @param reverse        (optional) Whether to reverse the waveform (true/false), default false
     * @param step           (optional) The waveform step value (0..7), default 0
     */
    void configureOscillator(buzzkill_osctype_t oscType,
                             byte oscNum,
                             double frequency,
                             buzzkill_shape_t shape,
                             byte midpoint=128,
                             bool invert=false,
                             bool reverse=false,
                             byte step=0);


    /**
     * Set the frequency for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param freqency       The oscillator frequency (0.0 - 4095.9375); will be set to next lowest 1/16th
     */
    void setFrequency(buzzkill_osctype_t oscType,
                      byte oscNum,
                      double frequency);


    /**
     * Set the midpoint for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param midpoint       The oscillator midpoint value (0..255)
     */
    void setMidpoint(buzzkill_osctype_t oscType,
                     byte oscNum,
                     byte midpoint);


    /**
     * Set the waveform shape for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param shape          The waveform shape (BUZZKILL_SHAPE_SINE, BUZZKILL_SHAPE_RAMP, etc.)
     */
    void setShape(buzzkill_osctype_t oscType,
                  byte oscNum,
                  buzzkill_shape_t setShape);


    /**
     * Set the invert flag for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param invert         Whether to invert the waveform (true/false)
     */
    void setInvert(buzzkill_osctype_t oscType,
                   byte oscNum,
                   bool invert);


    /**
     * Set the reverse flag for a specified oscillator.
     * The desired oscillator is specified by type and number.
     * @param oscType        The oscillator type (BUZZKILL_OSCTYPE_MOD or BUZZKILL_OSCTYPE_VOICE)
     * @param oscNum         The oscillator number (0..3) within the specified type
     * @param reverse        Whether to reverse the waveform (true/false)
     */
    void setReverse(buzzkill_osctype_t oscType,
                    byte oscNum,
                    bool reverse);


    /**
     * Restart one or more oscillators, according to the provided bit mask: 1=restart, 0=ignore.
     * A restarted oscillator will immediately return to its starting value and continue its cycle from the beginning.
     * @param restarttMask   A binary mask; bits 0-3 correspond to mod oscillators 0-3, bits 4-7 to voice oscillators 0-3
     */
    void restartOscillators(byte restartMask);


    /**
     * Halt one or more oscillators, according to the provided bit mask: 1=halted, 0=running.
     * A halted oscillator will be held at its starting value until it is un-halted.
     * @param haltMask       A binary mask; bits 0-3 correspond to mod oscillators 0-3, bits 4-7 to voice oscillators 0-3
     */
    void haltOscillators(byte haltMask);


    /**
     * Configure all options for a specified envelope.
     * All values are manually specified by the user.
     * @param envNum         The envelope number (0..3)
     * @param curveType      The curve type (BUZZKILL_CURVE_UP, BUZZKILL_CURVE_DOWN, etc.)
     * @param attackRange    The attack range (0-3)
     * @param attackVal      The attack value within the selected range (0..15)
     * @param decayRange     The decay range (0-3)
     * @param decayVal       The decay value within the selected range (0..15)
     * @param sustainLev     The sustain level while the note is on (0..127)
     * @param releaseRange   The release range (0..3)
     * @param releaseVal     The release value within the selected range (0..15)
     * @param mixVol         The envelope mix volume (0..15)
     * @param noteOn         Whether to gate the note on (true/false)
     */
    void configureEnvelope(byte envNum,
                           buzzkill_curve_t curveType,
                           byte attackRange,
                           byte attackVal,
                           byte decayRange,
                           byte decayVal,
                           byte sustainLev,
                           byte releaseRange,
                           byte releaseVal,
                           byte mixVol,
                           bool noteOn);


    /**
     * Configure all options for a specified envelope.
     * Time values are specified in ms and range/value options are calculated automatically.
     * @param envNum         The envelope number (0..3)
     * @param curveType      The curve type (BUZZKILL_CURVE_UP, BUZZKILL_CURVE_DOWN, etc.)
     * @param attackTime     The desried attack time in ms; the closest possible setting will be selected
     * @param decayTime      The desired decay time in ms; the closest possible setting will be selected
     * @param sustainLev     The sustain level while the note is on (0..127)
     * @param releaseTime    The desired release time in ms; the closest possible setting will be selected
     * @param mixVol         The envelope mix volume (0..15)
     * @param noteOn         Whether to gate the note on (true/false)
     */
    void configureEnvelope(byte envNum,
                           buzzkill_curve_t curveType,
                           word attackTime,
                           word decayTime,
                           byte sustainLev,
                           word releaseTime,
                           byte mixVol,
                           bool noteOn);


    /**
     * Set the curve type for a specified envelope.
     * @param envNum         The envelope number (0..3)
     * @param curveType      The curve type (BUZZKILL_CURVE_UP, BUZZKILL_CURVE_DOWN, etc.)
     */
    void setCurve(byte envNum,
                  buzzkill_curve_t curveType);


    /**
     * Set the attack rate for a specified envelope.
     * Values are manually specified by the user.
     * @param envNum         The envelope number (0..3)
     * @param attackRange    The attack range (0-3)
     * @param attackVal      The attack value within the selected range (0..15)
     */
    void setAttack(byte envNum,
                   byte attackRange,
                   byte attackVal);


    /**
     * Set the attack rate for a specified envelope.
     * Values are calculated automatically from a given time parameter.
     * @param envNum         The envelope number (0..3)
     * @param attackTime     The desried attack time in ms; the closest matching setting will be selected
     */
    void setAttack(byte envNum,
                   word attackTime);


    /**
     * Set the decay rate for a specified envelope.
     * Values are manually specified by the user.
     * @param envNum         The envelope number (0..3)
     * @param decayRange     The decay range (0-3)
     * @param decayVal       The decay value within the selected range (0..15)
     */
    void setDecay(byte envNum,
                  byte decayRange,
                  byte decayVal);


    /**
     * Set the decay rate for a specified envelope.
     * Values are calculated automatically from a given time parameter.
     * @param envNum         The envelope number (0..3)
     * @param decayTime      The desried decay time in ms; the closest matching setting will be selected
     */
    void setDecay(byte envNum,
                  word decayTime);


    /**
     * Set the sustain level for a specified envelope.
     * @param envNum         The envelope number (0..3)
     * @param sustainLev     The sustain level while the note is on (0..127)
     */
    void setSustain(byte envNum,
                    byte sustainLev);


    /**
     * Set the release rate for a specified envelope.
     * Values are manually specified by the user.
     * @param envNum         The envelope number (0..3)
     * @param releaseRange   The release range (0-3)
     * @param releaseVal     The release value within the selected range (0..15)
     */
    void setRelease(byte envNum,
                    byte releaseRange,
                    byte releaseVal);


    /**
     * Set the release rate for a specified envelope.
     * Values are calculated automatically from a given time parameter.
     * @param envNum         The envelope number (0..3)
     * @param releaseTime    The desried release time in ms; the closest matching setting will be selected
     */
    void setRelease(byte envNum,
                    word releaseTime);


    /**
     * Set the mix volume for a specified envelope.
     * @param envNum         The envelope number (0..3)
     * @param mixVol         The envelope mix volume (0..15)
     */
    void setMixVolume(byte envNum,
                      byte mixvol);


    /**
     * Set the gate bit for a specified envelope, turning the note on or off.
     * @param envNum         The envelope number (0..3)
     * @param noteOn         (optional) Whether to gate the note on or off (true/false); defaults to true
     */
    void noteOn(byte envNum,
                bool gate=true);


    /**
     * Set/Clear the gate bit for all envelopes, turning all notes on or off.
     * @param env0NoteOn     Whether to set or clear the gate bit for envelope 0 (true/false)
     * @param env1NoteOn     Whether to set or clear the gate bit for envelope 1 (true/false)
     * @param env2NoteOn     Whether to set or clear the gate bit for envelope 2 (true/false)
     * @param env3NoteOn     Whether to set or clear the gate bit for envelope 3 (true/false)
     */
    void noteOn(bool env0NoteOn,
                bool env1NoteOn,
                bool env2NoteOn,
                bool env3NoteOn);


    /**
     * Clear the gate bit for a specified envelope, turning the note on.
     * Equivalent to noteOn(envNum, false).
     * @param envNum         The envelope number (0..3)
     */
    void noteOff(byte envNum);


    /**
     * Set the master (global) volume level.
     * @param volume         The master volume level (0..15)
     */
    void setMasterVolume(byte volume);


    /**
     * Enable/Disable the audio output for a specific voice oscillator.
     * @param voiceNum       The voice oscillator number (0..3)
     * @param enable         (optional) Whether to enable or disable the audio output (true/false); defaults to true
     */
    void enableVoice(byte voiceNum,
                     bool enable=true);


    /**
     * Enable/Disable the audio output for all voice oscillators.
     * @param voice0Enable   Whether to enable or disable the output of voice oscillator 0 (true/false)
     * @param voice1Enable   Whether to enable or disable the output of voice oscillator 1 (true/false)
     * @param voice2Enable   Whether to enable or disable the output of voice oscillator 2 (true/false)
     * @param voice3Enable   Whether to enable or disable the output of voice oscillator 3 (true/false)
     */
    void enableVoice(bool voice0Enable,
                     bool voice1Enable,
                     bool voice2Enable,
                     bool voice3Enable);


    /**
     * Disable the audio output for a specific voice oscillator.
     * Equivalent to enableVoice(voiceNum, false).
     * @param voiceNum       The voice oscillator number (0..3)
     */
    void disableVoice(byte voiceNum);


    /**
     * Add a new modulation patch, automatically assigning one of five available slots (0..4).
     * Lowest available slot will always be assigned, so if all slots are unused sequence will be predictable: 0, 1, 2, 3, 4.
     * @param srcMod         The number of the modulation oscillator to be used as the patch source (0..3)
     * @param destVoice      The number of the voice oscillator to be used as the patch destination (0..3)
     * @param patchType      The patch type, e.g. BUZZKILL_PATCH_FREQSHIFT, BUZZKILL_PATCH_HARDSYNC, etc.
     * @param patchParam     The patch parameter value, exact meaning dependent on patch type (0..255)
     * @return               Slot number (0..4) if successful, 255 on failure (no slot available)
     */
    byte addPatch(byte srcMod,
                  byte destVoice,
                  buzzkill_patch_t patchType,
                  byte patchParam);


    /**
     * Remove a modulation patch.
     * @param patchSlot      The slot number of the patch to remove, assigned when the patch was added (0..4)
     */
    void removePatch(byte patchSlot);


    /**
     * Remove all modulation patches.
     */
    void clearPatches();


    /**
     * Clear speech buffer. This is the only way to remove phonemes from the buffer once added.
     */
    void clearSpeechBuffer();


    /**
     * Add new phonemes to the end of the speech buffer, pulling from an array of byte values.
     * The array may be terminated by a value of 0xff, or a total length may be specified.
     * @param phonemes       An array of phoneme byte values, possibly terminated by 0xff value
     * @param length         (optional) Number of phonemes to add, if array is unterminated
     */
    void addSpeechPhonemes(const byte phonemes[],
                           byte length=0);


    /**
     * Add new phonemes to the end of the speech buffer, pulling from a char array or string literal.
     * The array or string may be terminated by a value of 0xff, or a total length may be specified.
     * @param phonemes       An array of phoneme char values, or a string literal
     * @param length         (optional) Number of phonemes to add, if array/string is unterminated
     */
    void addSpeechPhonemes(const char phonemes[],
                           byte length=0);


    /**
     * Add phonemes to the end of the speech buffer, translating text tags from a char array or C-style string.
     * The array/string may be terminated by a period('.') or a null char (0x00), or a length may be specified.
     * @param tagss          A char array or string containing a sequence of two-letter text tags
     * @param length         (optional) Number of phonemes to add, if array/string is unterminated
     */
    void addSpeechTags(const char tags[],
                       byte length=0);


    /**
     * Convert a two-letter text tag into a numeric phoneme value
    */
    byte getPhonemeFromTag(const char tag[]);


    /**
     * Start speaking the phonemes currently in the speech buffer.
     * Speaking will continue until the end of the buffer is reached, or manually stopped.
     */
    void startSpeaking();


    /**
     * Stop speaking. Only needed if you wish to interrupt speaking before end of buffer is reached.
     */
    void stopSpeaking();


    /**
     * Sets the speed at which speech is articulated (0-253). Higher values result in faster speech.
     * @param speed          Speech speed value
     */
    void setSpeechSpeed(byte speed);


    /**
     * Sets the speech adjustment factors, which scale frequency and amplitutde components upward or downward.
     * There are four pairs of factors, controlling the frequency and amplitude for each of four speech formants.
     * @param form1Freq      Formant1 frequency adjustment factor
     * @param form1Amp       Formant1 amplitude adjustment factor
     * @param form2Freq      Formant2 frequency adjustment factor
     * @param form2Amp       Formant2 amplitude adjustment factor
     * @param form3Freq      Formant3 frequency adjustment factor
     * @param form3Amp       Formant3 amplitude adjustment factor
     * @param form4Freq      Formant4 frequency adjustment factor
     * @param form5Amp       Formant4 amplitude adjustment factor
     */
    void setSpeechFactors(byte form1Freq,
                          byte form1Amp,
                          byte form2Freq,
                          byte form2Amp,
                          byte form3Freq,
                          byte form3Amp,
                          byte form4Freq,
                          byte form4Amp);


    /**
     * Prepare speech mode by setting all appropriate registers to standard values for vocal output.
     * Sets the frequency/midpoint/waveform values for all voice oscillators and modulation oscillator 0.
     * Creates a modulation patch in slot 0, and clears all other patches.
     * Enables all voice outputs and sets mix volumes to 5/5/5/3 for envelopes 0/1/2/3 respectively.
     * Does not modify master volume, speech speed or speech factor settings.
     * @param pitch          Speech pitch value (in hertz)
     * @param patchType      Patch type for pitch modulation; e.g. BUZZKILL_PATCH_AMPSCALEMULTI or BUZZKILL_PATCH_HARDSYNCMULTI   
     */
    void prepareSpeechMode(double pitch,
                           buzzkill_patch_t patchType);


    /**
     * Put board into sleep mode, minimizing power requirements.
     */
    void boardSleep();

    /**
     * Wake board from sleep mode and return to normal operation.
     */
    void boardWake();


    /**
     * Define a custom waveform shape, consisting of 256 values each in the range (0..255).
     * @param wavedata       An array of bytes which define the waveform; must contain at least 256 values
     */
    void storeCustomWave(const byte wavedata[]);


    /**
     * Change the board I2C address. This is a hardware-level change, and affects all future I2C communication.
     * The new address will be automatically used by the current session, no need to call .beginI2C() again.
     * Any future calls to .beginI2C() must use the new address.
     * @param newAddr        The new 7-bit I2C address (8..119)
     */
    void changeI2CAddress(byte newAddr);

private:
    SPIClass *_spi=nullptr;
    TwoWire *_i2c=nullptr;
    byte _spiSS;
    byte _i2cAddr;
    byte _shadows[30];
    static constexpr char _phonlist[] PROGMEM = "OWAWEYAIAYEAOYURAEAAAUEHIYAOERAHUWUHIHAXS*SHF*V*Z*ZHTHDHM*N*NGH*X*R*RXL*LXW*WHY*WXYXKXGXT*D*P*B*K*G*J*CH_1_2_3";
    void _resetShadows(byte regStart);
    void _timeConvert(word time, byte &range, byte &value);
    void _send(byte command, byte data[], byte length);
};

#endif // BUZZKILL_H
