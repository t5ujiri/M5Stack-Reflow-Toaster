#include <Arduino.h>
#include <SPI.h>
#include "MAX31855.h"

int8_t MAX31855::begin(int8_t sck, int8_t miso, int8_t mosi, int8_t slave) {
  pinMode(slave, OUTPUT);
  digitalWrite(slave, HIGH);
  max31855.slave = slave;

  SPI.begin(sck, miso, mosi, slave);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE0);
  return 0;
}

int8_t MAX31855::readThermocoupleTemperature() {
  // 14 bit thermocouple temperature data + 2 bit
  unsigned int thermocouple;
  // 12 bit internal temperature data + 4 bit
  unsigned int internal;
  digitalWrite(max31855.slave, LOW);
  thermocouple = (unsigned int)SPI.transfer(0x00) << 8;
  thermocouple |= (unsigned int)SPI.transfer(0x00);
  internal = (unsigned int)SPI.transfer(0x00) << 8;
  internal |= (unsigned int)SPI.transfer(0x00);
  digitalWrite(max31855.slave, HIGH);

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
    if ((thermocouple & 0x8000) == 0) {
      max31855.thermocoupleTemp = (thermocouple >> 2) * 0.25;
    } else {
      max31855.thermocoupleTemp = (0x3fff - (thermocouple >> 2) + 1) * -0.25;
    }
    return 0;
  }
}

MAX31855 max31855;