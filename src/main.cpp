#include <Arduino.h>
#include <M5Stack.h>
#include <MAX31855.h>
#include <SPI.h>

#define MAX_PROCESS_DURATION 240
#define THERMO_COUPLE_SCK 18
#define THERMO_COUPLE_MISO 19
#define THERMO_COUPLE_MOSI 23
#define THERMO_COUPLE_SLAVE 2
#define HEATER1 16
#define HEATER2 17
bool isProcessing;

void StandbyRoutine();
void ProcessRoutine();
void DrawAxis();
void IRAM_ATTR OnTimer();

hw_timer_t *timer;
unsigned int timerCount;
int interruptCounter;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timeMux = portMUX_INITIALIZER_UNLOCKED;

void setup() {
  M5.begin(true, false);
  M5.Speaker.setVolume(1);
  M5.Speaker.setBeep(440, 300);
  max31855.begin(THERMO_COUPLE_SCK, THERMO_COUPLE_MISO, THERMO_COUPLE_MOSI,
                 THERMO_COUPLE_SLAVE);
  // put your setup code here, to run once:
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &OnTimer, true);
  timerAlarmWrite(timer, 1000000, true);

  timerSemaphore = xSemaphoreCreateBinary();
}

void loop() {
  M5.Lcd.clearDisplay();
  while (true) {
    StandbyRoutine();
    m5.update();
    if (m5.BtnB.wasPressed()) {
      break;
    }
    vTaskDelay(1);
  }

  isProcessing = true;
  M5.Lcd.clearDisplay();
  DrawAxis();
  timerAlarmEnable(timer);
  m5.Speaker.beep();

  timerCount = 0;
  while (isProcessing) {
    m5.update();
    if (interruptCounter > 0) {
      portENTER_CRITICAL(&timeMux);
      interruptCounter--;
      portEXIT_CRITICAL(&timeMux);
      ProcessRoutine();
    }
    vTaskDelay(1);
  }
  timerAlarmDisable(timer);
}

void StandbyRoutine() {
  M5.Lcd.drawString("StandBy", 50, 0);
  M5.Lcd.drawString("Press center button to start", 50, 10);
}

void IRAM_ATTR OnTimer() {
  portENTER_CRITICAL_ISR(&timeMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timeMux);
}

void ProcessRoutine() {
  if (max31855.readThermocoupleTemperature() > 0) {
    M5.Lcd.drawString("error!", 0, 0);
    m5.Speaker.beep();
    isProcessing = false;
  } else if (timerCount > MAX_PROCESS_DURATION) {
    m5.Speaker.beep();
    isProcessing = false;
  } else {
    M5.Lcd.drawString(String(timerCount, DEC) + " / " + String(max31855.thermocoupleTemp, DEC), 50, 0);
    m5.Lcd.drawLine(20 + timerCount, 240 - 20 - (int)max31855.thermocoupleTemp,
                    20 + timerCount, 240 - 20 - (int)max31855.thermocoupleTemp,
                    TFT_BLUE);
    timerCount++;
  }
}

void DrawAxis() {
  m5.Lcd.drawLine(20, 20, 20, 220, TFT_WHITE);
  m5.Lcd.drawLine(20, 220, 300, 220, TFT_WHITE);
}