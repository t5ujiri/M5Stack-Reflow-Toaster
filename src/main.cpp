#include <Arduino.h>
#include <M5Stack.h>
#include <MAX31855.h>
#include <SPI.h>
#include <ReflowProfile.h>

#define MAX_PROCESS_DURATION 240
#define THERMO_COUPLE_SCK 18
#define THERMO_COUPLE_MISO 19
#define THERMO_COUPLE_MOSI 23
#define THERMO_COUPLE_SLAVE 2
#define HEATER1 16
#define HEATER2 17

// axis profile
const uint16_t tempMax = 300;
const uint16_t timeMax = 280;
const uint16_t axisXLength = 280;
const uint16_t axisYLength = 200;
const uint16_t axisXOrigin = 30;
const uint16_t axisYOrigin = 220;

bool isProcessing;
unsigned int timerCount;
void StandbyRoutine();
void ProcessRoutine();
void DrawAxis();
void DrawLineOnGraph(float x0, float y0, float x1, float y1, uint32_t color);

#define allOFF 0
#define upON   1
#define downON 2
#define allON  3
int temperature_control_data[][3] = {
  {downON, 130, 15},  // 2ON   , temperature130, keep15sec
  {allON , 230,  0},  // 1&2ON , temperature230, keep0sec
  {downON, 225,100},  // 2ON   , temperature225, keep100sec
  {allOFF,   0,  0}   // 1&2OFF, temperature0  , keep0sec
};

void setup() {
  // M5Stack Setup
  M5.begin(true, false);
  M5.Speaker.setVolume(1);
  M5.Speaker.setBeep(440, 300);

  // Thermocouple setup
  max31855.begin(THERMO_COUPLE_SCK, THERMO_COUPLE_MISO, THERMO_COUPLE_MOSI,
                 THERMO_COUPLE_SLAVE);

  // heat control setup
  pinMode(HEATER1, OUTPUT);
  pinMode(HEATER2, OUTPUT);
  digitalWrite(HEATER1, LOW);
  digitalWrite(HEATER2, LOW);
}

void loop() {
  // standby
  while (!isProcessing) {
    m5.update();
    StandbyRoutine();
    vTaskDelay(1);
  }

  // starting process
  m5.Lcd.clear();
  DrawAxis();
  M5.Lcd.drawString("Press center button to stop", 50, 10);

  // draw reflow profile

  // in process
  while (isProcessing) {
    m5.update();
    ProcessRoutine();
    vTaskDelay(1000);
  }
}

void StandbyRoutine() {
  M5.Lcd.drawString("StandBy", 50, 0);
  M5.Lcd.drawString("Press center button to start", 50, 10);
  if (M5.BtnB.wasPressed()) {
    isProcessing = true;
  }
}

void ProcessRoutine() {
  if (max31855.readThermocoupleTemperature() > 0) {
    M5.Lcd.drawString("error!", 0, 0);
    timerCount = 0;
    isProcessing = false;
  } else if (timerCount > MAX_PROCESS_DURATION) {
    timerCount = 0;
    isProcessing = false;
  } else if (M5.BtnB.wasPressed()) {
    timerCount = 0;
    isProcessing = false;
  } else {
    M5.Lcd.drawString(String(timerCount, DEC) + " / " +
                          String(max31855.thermocoupleTemp, DEC),
                      150, 0);
    DrawLineOnGraph(timerCount, max31855.thermocoupleTemp, timerCount,
                    max31855.thermocoupleTemp, TFT_BLUE);
    timerCount++;
  }
}

void DrawAxis() {
  // x axis
  m5.Lcd.drawLine(axisXOrigin, axisYOrigin, axisXOrigin + axisXLength,
                  axisYOrigin, TFT_WHITE);
  // y axis
  m5.Lcd.drawLine(axisXOrigin, axisYOrigin, axisXOrigin,
                  axisYOrigin - axisYLength, TFT_WHITE);
  for (int i = 0; i < 13; i++) {
    m5.Lcd.drawString(
        String((uint16_t)(20 * i / 10), DEC),
        axisXOrigin + (int)((float)axisXLength / timeMax * 20 * i), 230);
  }
  for (int i = 0; i < 7; i++) {
    m5.Lcd.drawString(
        String((uint16_t)(50 * i), DEC), 0,
        axisYOrigin - (int)((float)axisYLength / tempMax * 50 * i));
  }
}

void DrawLineOnGraph(float x0, float y0, float x1, float y1, uint32_t color) {
  m5.Lcd.drawLine(axisXOrigin + (int)((float)axisXLength / timeMax * x0),
                  axisYOrigin - (int)((float)axisYLength / tempMax * y0),
                  axisXOrigin + (int)((float)axisXLength / timeMax * x1),
                  axisYOrigin - (int)((float)axisYLength / tempMax * y1),
                  color);
}

void SwitchHeater(uint8_t up, uint8_t down){
  digitalWrite(HEATER1, up);
  digitalWrite(HEATER1, down);
}

bool OnOffControl(int currentTime, ReflowProfile profile){
  
}