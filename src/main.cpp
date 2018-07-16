#include <Arduino.h>
#include <SPI.h>

#define THERMO_COUPLE_SLAVE 10

void setup()
{
    // put your setup code here, to run once:
    pinMode(THERMO_COUPLE_SLAVE, OUTPUT);
    digitalWrite(THERMO_COUPLE_SLAVE, HIGH);

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.setDataMode(SPI_MODE0);
}

void loop()
{
    // 14 bit thermocouple temperature data + 2 bit
    unsigned int thermocouple;
    // 12 bit internal temperature data + 4 bit
    unsigned int internal;
    float disp;

    delay(500);
    digitalWrite(THERMO_COUPLE_SLAVE, LOW);
    thermocouple = (unsigned int)SPI.transfer(0x00) << 8;
    thermocouple |= (unsigned int)SPI.transfer(0x00);
    internal = (unsigned int)SPI.transfer(0x00) << 8;
    internal |= (unsigned int)SPI.transfer(0x00);
}
