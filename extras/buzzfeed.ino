#include <SPI.h>
#include <Wire.h>

constexpr bool USE_I2C = true;
constexpr byte ADDR_I2C = 10;
constexpr byte SS_PIN = 10;

int dataMode=-1, dataIndex=0;
byte dataBuffer[300];

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  if (USE_I2C) {
    Wire.begin();
    Wire.setClock(400000);    //Sets clock for I2C communication at 400 kHz
  }
  else {
    pinMode(SS_PIN, OUTPUT);
    digitalWrite(SS_PIN, HIGH);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV32);    //Sets clock for SPI communication at 500 kHz (assuming 16 MHz system clock)
  }
}

void loop() {
  byte dataInput;
  if (Serial.available() <= 0) return;
  dataInput = Serial.read();
  if (dataMode<0) {
    if (dataInput == 90) dataMode = 0;
    return;
  }
  else if (dataMode == 0) {
    dataMode = dataInput;
    dataIndex = 0;
    return;
  }
  dataBuffer[dataIndex++]=dataInput;
  if (dataIndex < dataMode) return;
  dataMode=-1;
  int dataPtr = 0;
  if (USE_I2C) {
    int dataCount;
    while (dataPtr < dataIndex) {
      Wire.beginTransmission(ADDR_I2C);
      for (dataCount = 0; dataCount < 32 && dataPtr < dataIndex; dataCount++, dataPtr++) Wire.write(dataBuffer[dataPtr]);
      Wire.endTransmission();
    }
  }
  else {
    digitalWrite(SS_PIN, LOW);
    for (; dataPtr < dataIndex; dataPtr++) SPI.transfer(dataBuffer[dataPtr]);
    digitalWrite(SS_PIN, HIGH);
  }
}
