#include <BuzzKill.h>

BuzzKill::BuzzKill() {
  _resetShadows(0);
}

void BuzzKill::beginSPI(byte pinSS=SS, SPIClass &spi=SPI) {
  _spiSS = pinSS;
  _spi = &spi;
}

void BuzzKill::beginI2C(byte address=10, TwoWire &wire=Wire) {
  _i2cAddr = address;
  _i2c = &wire;
}

void BuzzKill::writeRegisters(byte regStart, byte regData[], byte length) {
  if (length < 1 || regStart > 60-length) return;
  for (byte reg=regStart, count=0; reg<regStart+length; ++reg, ++count) {
    if (reg > 48) {
      if ((reg % 2) == 0) _shadows[reg/2] = regData[count];
    }
    else if (reg > 31) _shadows[reg-24] = regData[count];
    else if ((reg % 4) == 3) _shadows[reg/4] = regData[count];
  }
  _send(regStart, regData, length);
}

void BuzzKill::writeRegisters(byte regStart, char regData[], byte length) {
  writeRegisters(regStart, (byte *)regData, length);
}

void BuzzKill::setRegister(byte regStart, byte val1, int val2=-1, int val3=-1, int val4=-1, int val5=-1, int val6=-1, int val7=-1, int val8=-1, int val9=-1, int val10=-1) {
  int arr16[] = {val1, val2, val3, val4, val5, val6, val7, val8, val9, val10};
  byte reg=regStart, count, arr8[10];
  for (count=0; count<10 && arr16[count]>=0; ++count, ++reg) {
    arr8[count] = arr16[count];
    if (reg > 48) {
      if ((reg % 2) == 0) _shadows[reg/2] = arr8[count];
    }
    else if (reg > 31) _shadows[reg-24] = arr8[count];
    else if ((reg % 4) == 3) _shadows[reg/4] = arr8[count];
  }
  _send(regStart, arr8, count);
}

void BuzzKill::resetRegisters(byte regStart=0) {
  if (regStart > 59) return;
  _resetShadows(regStart);
  _send(regStart, nullptr, 0);
}

void BuzzKill::configureOscillator(buzzkill_osctype_t oscType, byte oscNum, double frequency, buzzkill_shape_t shape, byte midpoint=128, bool invert=false, bool reverse=false, byte step=0) {
  if (oscNum>3  || step>7 || frequency>=4096) return;
  word wfreq = frequency * 16;
  byte arr[4] = { wfreq & 255, wfreq >> 8, midpoint, shape | (reverse?16:0) | (invert?8:0) | step };
  _shadows[(oscType>>2)+oscNum]=arr[3];
  _send(oscType+(oscNum<<2), arr, 4);
}

void BuzzKill::setFrequency(buzzkill_osctype_t oscType, byte oscNum, double frequency) {
  if (oscNum>3  || frequency>=4096) return;
  word wfreq = frequency * 16;
  byte arr[2] = { wfreq & 255, wfreq >> 8 };
  _send(oscType+(oscNum<<2), arr, 2);
}

void BuzzKill::setMidpoint(buzzkill_osctype_t oscType, byte oscNum, byte midpoint) {
  if (oscNum>3) return;
  _send(oscType+(oscNum<<2)+2, &midpoint, 1);
}

void BuzzKill::setShape(buzzkill_osctype_t oscType, byte oscNum, buzzkill_shape_t setShape) {
  if (oscNum>3) return;
  byte index = (oscType>>2) + oscNum;
  _shadows[index] = (_shadows[index] & 31) | setShape;
  _send(oscType+(oscNum<<2)+3, &_shadows[index], 1);
}

void BuzzKill::setInvert(buzzkill_osctype_t oscType, byte oscNum, bool invert) {
  if (oscNum>3) return;
  byte index = (oscType>>2) + oscNum;
  _shadows[index] = (_shadows[index] & (~8)) | (invert?8:0);
  _send(oscType+(oscNum<<2)+3, &_shadows[index], 1);
}

void BuzzKill::setReverse(buzzkill_osctype_t oscType, byte oscNum, bool reverse) {
  if (oscNum>3) return;
  byte index = (oscType>>2) + oscNum;
  _shadows[index] = (_shadows[index] & (~16)) | (reverse?16:0);
  _send(oscType+(oscNum<<2)+3, &_shadows[index], 1);
}

void BuzzKill::restartOscillators(byte restartMask) {
  _send(248, &restartMask, 1);
}

void BuzzKill::haltOscillators(byte haltMask) {
  _send(49, &haltMask, 1);
}

void BuzzKill::configureEnvelope(byte envNum, buzzkill_curve_t curveType, byte attackRange, byte attackVal, byte decayRange, byte decayVal, byte sustainLev, byte releaseRange, byte releaseVal, byte mixVol, bool noteOn) {
  if (envNum>3 || attackRange>3 || attackVal>15 || decayRange>3 || decayVal>15 || sustainLev>127 || releaseRange>3 || releaseVal>15 || mixVol>15) return;
  byte arr[4] = { curveType | (releaseRange<<4) | (decayRange<<2) | attackRange, (decayVal<<4) | attackVal, (noteOn?128:0) | sustainLev, (mixVol<<4) | releaseVal };
  for (byte x=0; x<4; ++x) _shadows[(envNum<<2)+8+x] = arr[x];
  _send((envNum<<2)+32, arr, 4);
}

void BuzzKill::configureEnvelope(byte envNum, buzzkill_curve_t curveType, word attackTime, word decayTime, byte sustainLev, word releaseTime, byte mixVol, bool noteOn) {
  byte attackRange, attackVal, decayRange, decayVal, releaseRange, releaseVal;
  _timeConvert(attackTime, attackRange, attackVal);
  _timeConvert(decayTime, decayRange, decayVal);
  _timeConvert(releaseTime, releaseRange, releaseVal);
  configureEnvelope(envNum, curveType, attackRange, attackVal, decayRange, decayVal, sustainLev, releaseRange, releaseVal, mixVol, noteOn);
}

void BuzzKill::setCurve(byte envNum, buzzkill_curve_t curveType) {
  if (envNum > 3) return;
  byte index = (envNum<<2) + 8;
  _shadows[index] = (_shadows[index] & 0b00111111) | curveType;
  _send((envNum<<2)+32, &_shadows[index], 1);
}

void BuzzKill::setAttack(byte envNum, byte attackRange, byte attackVal) {
  if (envNum>3 || attackRange>3 || attackVal>15) return;
  byte index = (envNum<<2) + 8;
  _shadows[index] = (_shadows[index] & 0b11111100) | attackRange;
  _shadows[index+1] = (_shadows[index+1] & 0b11110000) | attackVal;
  _send((envNum<<2)+32, &_shadows[index], 2);
}

void BuzzKill::setAttack(byte envNum, word attackTime) {
  byte attackRange, attackVal;
  _timeConvert(attackTime, attackRange, attackVal);
  setAttack(envNum, attackRange, attackVal);
}

void BuzzKill::setDecay(byte envNum, byte decayRange, byte decayVal) {
  if (envNum > 3 || decayRange > 3 || decayVal > 15) return;
  byte index = (envNum<<2) + 8;
  _shadows[index] = (_shadows[index] & 0b11110011) | (decayRange<<2);
  _shadows[index+1] = (_shadows[index+1] & 0b00001111) | (decayVal<<4);
  _send((envNum<<2)+32, &_shadows[index], 2);
}

void BuzzKill::setDecay(byte envNum, word decayTime) {
  byte decayRange, decayVal;
  _timeConvert(decayTime, decayRange, decayVal);
  setDecay(envNum, decayRange, decayVal);
}

void BuzzKill::setSustain(byte envNum, byte sustainLev) {
  if (envNum > 3 || sustainLev > 127) return;
  byte index = (envNum<<2) + 10;
  _shadows[index] = (_shadows[index] & 0b10000000) | sustainLev;
  _send((envNum<<2)+34, &_shadows[index], 1);
}

void BuzzKill::setRelease(byte envNum, byte releaseRange, byte releaseVal) {
  if (envNum > 3 || releaseRange > 3 || releaseVal > 15) return;
  byte index = (envNum<<2) + 8;
  _shadows[index] = (_shadows[index] & 0b11001111) | (releaseRange<<4);
  _shadows[index+3] = (_shadows[index+3] & 0b11110000) | releaseVal;
  _send((envNum<<2)+32, &_shadows[index], 1);
  _send((envNum<<2)+35, &_shadows[index+3], 1);
}

void BuzzKill::setRelease(byte envNum, word releaseTime) {
  byte releaseRange, releaseVal;
  _timeConvert(releaseTime, releaseRange, releaseVal);
  setRelease(envNum, releaseRange, releaseVal);
}

void BuzzKill::setMixVolume(byte envNum, byte mixVol) {
  if (envNum > 3 || mixVol > 15) return;
  byte index = (envNum<<2) + 11;
  _shadows[index] = (_shadows[index] & 0b00001111) | (mixVol<<4);
  _send((envNum<<2)+35, &_shadows[index], 1);
}

void BuzzKill::noteOn(byte envNum, bool noteOn=true) {
  if (envNum > 3) return;
  byte index = (envNum<<2) + 10;
  if (noteOn) _shadows[index] |= 128; else _shadows[index] &= (~128);
  _send((envNum<<2)+34, &_shadows[index], 1);
}

void BuzzKill::noteOn(bool env0NoteOn, bool env1NoteOn, bool env2NoteOn, bool env3NoteOn) {
  byte index;
  bool arr[] = { env0NoteOn, env1NoteOn, env2NoteOn, env3NoteOn };
  for (byte x=0; x<4; ++x) {
    index = (x<<2) + 10;
    if (arr[x] && _shadows[index] < 128) {
      _shadows[index] |= 128;
      _send((x<<2)+34, &_shadows[index], 1);
    }
    else if (!arr[x] && _shadows[index] > 127) {
      _shadows[index] &= (~128);
      _send((x<<2)+34, &_shadows[index], 1);
    }
  }
}

void BuzzKill::noteOff(byte envNum) {
  noteOn(envNum, false);
}

void BuzzKill::setMasterVolume(byte volume) {
  if (volume > 15) return;
  _shadows[24] = (_shadows[24] & 0b00001111) | (volume<<4);
  _send(48, &_shadows[24], 1);
}

void BuzzKill::enableVoice(byte voiceNum, bool enable=true) {
  if (voiceNum > 3) return;
  if (enable) _shadows[24] |= (1<<voiceNum); else _shadows[24] &= ~(1<<voiceNum);
  _send(48, &_shadows[24], 1);
}

void BuzzKill::enableVoice(bool voice0Enable, bool voice1Enable, bool voice2Enable, bool voice3Enable) {
  byte mask = (voice3Enable?8:0) | (voice2Enable?4:0) | (voice1Enable?2:0) | (voice0Enable?1:0);
  _shadows[24] = (_shadows[24] & 0b11110000) | mask;
  _send(48, &_shadows[24], 1);
}

void BuzzKill::disableVoice(byte voiceNum) {
  enableVoice(voiceNum, false);
}

byte BuzzKill::addPatch(byte srcMod, byte destVoice, buzzkill_patch_t patchType, byte patchParam) {
  if (srcMod > 3 || destVoice > 3 || patchType > 15) return 255;
  byte slot, arr[2];
  for (slot=0; slot<5; ++slot) if ((_shadows[slot+25] & 0b00001111) == 0) break;
  if (slot > 4) return 255;
  arr[0] = _shadows[slot+25] = (destVoice<<6) | (srcMod<<4) | patchType;
  arr[1] = patchParam;
  _send((slot<<1)+50, arr, 2);
  return slot;
}

void BuzzKill::removePatch(byte patchSlot) {
  if (patchSlot > 4) return;
  _shadows[patchSlot+25] = 0;
  _send((patchSlot<<1)+50, &_shadows[patchSlot+25], 1);
}

void BuzzKill::clearPatches() {
  resetRegisters(50);
}

void BuzzKill::clearSpeechBuffer() {
  _send(60, nullptr, 0);
}

void BuzzKill::addSpeechPhonemes(const byte phonemes[], byte length=0) {
  if (length == 0) for (; length<255; ++length) if (phonemes[length] == 255) break;
  if (length < 255) _send(60, phonemes, length);
}

void BuzzKill::addSpeechPhonemes(const char phonemes[], byte length=0) {
  addSpeechPhonemes((byte *)phonemes, length);
}

void BuzzKill::addSpeechTags(const char tags[], byte length=0) {
  char *tagptr = tags;
  word count = 0;
  if (length == 0) {
    while (count < 509 && *tagptr && *tagptr != '.') if (*tagptr++ != ' ') ++count;
    if (count == 0 || (count%2) == 1) return;
    length = count>>1;
    tagptr = tags;
    count = 0;
  }
  byte arr[length];
  while (count < length) {
    while (*tagptr == ' ') ++tagptr;
    arr[count++] = getPhonemeFromTag(tagptr);
    tagptr+=2;
  }
  _send(60, arr, length);
}

byte BuzzKill::getPhonemeFromTag(const char tag[]) {
  byte listIndex;
  for (listIndex = 0; listIndex < 110; listIndex += 2) if (!strncasecmp_PF(tag, _phonlist + listIndex, 2)) break;
  return listIndex >> 1;
}

void BuzzKill::startSpeaking() {
  _send(247, nullptr, 0);
}

void BuzzKill::stopSpeaking() {
  _send(246, nullptr, 0);
}

void BuzzKill::setSpeechSpeed(byte speed) {
  if (speed > 253) return;
  _send(244, &speed, 1);
}

void BuzzKill::setSpeechFactors(byte form1Freq, byte form1Amp, byte form2Freq, byte form2Amp, byte form3Freq, byte form3Amp, byte form4Freq, byte form4Amp) {
  byte arr[] = { form1Freq, form1Amp, form2Freq, form2Amp, form3Freq, form3Amp, form4Freq, form4Amp };
  _send(245, arr, 8);
}

void BuzzKill::prepareSpeechMode(double pitch, buzzkill_patch_t patchType) {
  configureOscillator(BUZZKILL_OSCTYPE_VOICE, 0, 0.0, BUZZKILL_SHAPE_SINE);
  configureOscillator(BUZZKILL_OSCTYPE_VOICE, 1, 0.0, BUZZKILL_SHAPE_SINE);
  configureOscillator(BUZZKILL_OSCTYPE_VOICE, 2, 0.0, BUZZKILL_SHAPE_SINE);
  configureOscillator(BUZZKILL_OSCTYPE_VOICE, 3, 0.0, BUZZKILL_SHAPE_NOISE);
  configureOscillator(BUZZKILL_OSCTYPE_MOD, 0, pitch, BUZZKILL_SHAPE_HILLTOP);
  setMixVolume(0, 5);
  setMixVolume(1, 5);
  setMixVolume(2, 5);
  setMixVolume(3, 3);
  clearPatches();
  addPatch(0, 0, patchType, 255);
  enableVoice(true, true, true, true);
}

void BuzzKill::boardSleep() {
  _send(251, nullptr, 0);
}

void BuzzKill::boardWake() {
  if (_spi) {
    digitalWrite(_spiSS, LOW);
    delay(1);
    digitalWrite(_spiSS, HIGH);
  }
  else {
    _i2c->beginTransmission(_i2cAddr);
    _i2c->write(255);
    _i2c->endTransmission();
  }
}

void BuzzKill::storeCustomWave(const byte wavedata[]) {
  _send(249, wavedata, 128);
  _send(255, wavedata+128, 128);
}

void BuzzKill::changeI2CAddress(byte newAddr) {
  if (newAddr < 8 || newAddr > 119) return;
  byte arr[] = { newAddr, newAddr ^ 0b01010101, newAddr ^ 0b10101010 };
  _send(250, arr, 3);
  _i2cAddr = newAddr;
}

void BuzzKill::_resetShadows(byte regStart) {
  byte index;
  for (index = 0; index < 8; ++index) if (regStart <= (index<<2) + 3) _shadows[index] = 0;
  for (index = 0; index < 16; index += 4) if (regStart <= index + 32) _shadows[index + 8] = 0;
  for (index = 1; index < 16; index += 4) if (regStart <= index + 32) _shadows[index + 8] = 0;
  for (index = 2; index < 16; index += 4) if (regStart <= index + 32) _shadows[index + 8] = 127;
  for (index = 3; index < 16; index += 4) if (regStart <= index + 32) _shadows[index + 8] = 240;
  if (regStart <= 48) _shadows[24] = 240;
  for (index = 0; index < 5; ++index) if (regStart <= (index<<1) + 50) _shadows[index + 25] = 0;
}

void BuzzKill::_send(byte command, byte data[], byte length) {
  byte extra = 255;
  if (command < 61) {
    command <<= 2;
    if (length > 0 && length < 4) command += length; else extra = length;
  }
  if (_spi) {
    _spi->beginTransaction(SPISettings(BUZZKILL_SPI_SPEED, MSBFIRST, SPI_MODE0));
    digitalWrite(_spiSS, LOW);
    if (command != 255) _spi->transfer(command);
    if (extra != 255) _spi->transfer(extra);
    if (length > 0) _spi->transfer(data, length);
    digitalWrite(_spiSS, HIGH);
    _spi->endTransaction();
  }
  else {
    word count = (extra==255 ? 31 : 30);
    if (length < count) count = length;
    _i2c->beginTransmission(_i2cAddr);
    if (command != 255) _i2c->write(command);
    if (extra != 255) _i2c->write(extra);
    while (length > 0) {
      _i2c->write(data, count);
      if (length == count) break;
      _i2c->endTransmission(false);
      data += count;
      length -= count;
      count = (length<32 ? length : 32);
    _i2c->beginTransmission(_i2cAddr);
    }
  _i2c->endTransmission(true);
  }
}

void BuzzKill::_timeConvert(word time, byte &range, byte &value) {
  if (time >= 5000) { range = 3; value = 15; }
  else if (time >= 1478) { range = 3; value = (time-1478) / 231; }
  else if (time >= 490) { range = 2; value = (time-490) / 62; }
  else if (time >= 123) { range = 1; value = (time-123) / 23; }
  else { range = 0; value = (time+1) / 8; }
}

constexpr char BuzzKill::_phonlist[];