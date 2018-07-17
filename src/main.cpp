#include <Arduino.h>
#include <M5Stack.h>
#include <SPI.h>
#define THERMO_COUPLE_SLAVE 21
float thermocoupleTemp, internalTemp;
int ReadThermocoupleValue();

void setup() {
  M5.begin(true, false);
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(THERMO_COUPLE_SLAVE, OUTPUT);
  digitalWrite(THERMO_COUPLE_SLAVE, HIGH);

  SPI.begin(18, 19, 23, 21);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE0);
}

void loop() {
  M5.Lcd.clearDisplay();
  if (ReadThermocoupleValue() > 0) {
    M5.Lcd.drawString("error!", 0, 0);
  } else {
    M5.Lcd.drawString(String(thermocoupleTemp, DEC), 50, 100);
    M5.Lcd.drawString(String(internalTemp, DEC), 50, 50);
  }
  delay(500);
}

int ReadThermocoupleValue() {
  // 14 bit thermocouple temperature data + 2 bit
  unsigned int thermocouple;
  // 12 bit internal temperature data + 4 bit
  unsigned int internal;
  digitalWrite(THERMO_COUPLE_SLAVE, LOW);
  thermocouple = (unsigned int)SPI.transfer(0x00) << 8;
  thermocouple |= (unsigned int)SPI.transfer(0x00);
  internal = (unsigned int)SPI.transfer(0x00) << 8;
  internal |= (unsigned int)SPI.transfer(0x00);
  digitalWrite(THERMO_COUPLE_SLAVE, HIGH);

  if ((thermocouple & 0x0001) != 0) {
    Serial.print("ERROR: ");
    if ((internal & 0x0004) != 0) {
      Serial.print("Short to Vcc, ");
      return 1;
    }
    if ((internal & 0x0002) != 0) {
      Serial.print("Short to GND, ");
      return 2;
    }
    if ((internal & 0x0001) != 0) {
      Serial.print("Open Circuit, ");
      return 3;
    }
    Serial.println();
  } else {
    if ((thermocouple & 0x8000) == 0) {  // 0℃以上   above 0 Degrees Celsius
      thermocoupleTemp = (thermocouple >> 2) * 0.25;
    } else {  // 0℃未満   below zero
      thermocoupleTemp = (0x3fff - (thermocouple >> 2) + 1) * -0.25;
    }

    if ((internal & 0x8000) == 0) {  // 0℃以上   above 0 Degrees Celsius
      internalTemp = (internal >> 4) * 0.0625;
    } else {  // 0℃未満   below zero
      internalTemp = (((0xffff - internal) >> 4) + 1) * -0.0625;
    }
    return 0;
  }
}