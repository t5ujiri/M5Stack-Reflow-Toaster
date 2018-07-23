#include <Arduino.h>
#include <M5Stack.h>
#include <MAX31855.h>
#include <ReflowProfile.h>
#include <SPI.h>

#define MAX_PROCESS_DURATION 240

// axis profile
const uint16_t tempMax = 300;
const uint16_t timeMax = 280;
const uint16_t axisXLength = 280;
const uint16_t axisYLength = 200;
const uint16_t axisXOrigin = 30;
const uint16_t axisYOrigin = 220;

bool isProcessing;
unsigned int timerCount;
ReflowProfile *reflowProfile;
void StandbyRoutine();
void ProcessRoutine();
void DrawAxis();
void DrawLineOnGraph(float x0, float y0, float x1, float y1, uint32_t color);

void setup() {
  // M5Stack Setup
  M5.begin(true, false);
  M5.Speaker.setVolume(1);

  // Thermocouple setup
  max31855.begin(THERMO_COUPLE_SCK, THERMO_COUPLE_MISO, THERMO_COUPLE_MOSI,
                 THERMO_COUPLE_SLAVE);

  // heat control setup
  reflowProfile = new ReflowProfile();
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
  reflowProfile->Initialize();

  // in process
  while (isProcessing) {
    m5.update();
    ProcessRoutine();
    vTaskDelay(1000);
  }
}

void StandbyRoutine() {
  M5.Lcd.drawString("StandBy    ", 0, 0);
  M5.Lcd.drawString("Press center button to start", 50, 10);
  if (M5.BtnB.wasPressed()) {
    isProcessing = true;
  }
}

void ProcessRoutine() {
  // thermocouple error
  if (max31855.readThermocoupleTemperature() > 0) {
    M5.Lcd.drawString("error", 0, 0);
    timerCount = 0;
    isProcessing = false;
    reflowProfile->Stop();
    return;
  }

  // exceed max process time
  if (timerCount > MAX_PROCESS_DURATION) {
    timerCount = 0;
    isProcessing = false;
    reflowProfile->Stop();
    return;
  }

  // cancel button pressed
  if (M5.BtnB.wasPressed()) {
    M5.Lcd.drawString("cancelled", 0, 0);
    timerCount = 0;
    reflowProfile->Stop();
    isProcessing = false;
    return;
  }

  M5.Lcd.drawString(
      String(timerCount, DEC) + " / " + String(max31855.thermocoupleTemp, 4),
      150, 0);
  DrawLineOnGraph(timerCount, max31855.thermocoupleTemp, timerCount,
                  max31855.thermocoupleTemp, TFT_BLUE);
  timerCount++;

  reflowProfile->ControlRoutine(max31855.thermocoupleTemp);
  if (reflowProfile->IsFinished()) {
    isProcessing = false;
    reflowProfile->Stop();
  }
  if (reflowProfile->IsIndexChanged()) {
    m5.Speaker.tone(441, 200);
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