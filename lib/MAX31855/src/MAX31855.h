#ifndef MAX31855_H
#define MAX31855_H
#define THERMO_COUPLE_SCK 18
#define THERMO_COUPLE_MISO 19
#define THERMO_COUPLE_MOSI 23
#define THERMO_COUPLE_SLAVE 2
#include <Arduino.h>
class MAX31855 {
 public:
  int8_t slave;
  float thermocoupleTemp, internalTemp;

 public:
  int8_t begin(int8_t sck, int8_t miso, int8_t mosi, int8_t slave);
  int8_t readThermocoupleTemperature();
};
extern MAX31855 max31855;
#endif