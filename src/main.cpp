#include <Arduino.h>
#include <M5Stack.h>
#include <SPI.h>
#define THERMO_COUPLE_SCK 18
#define THERMO_COUPLE_MISO 19
#define THERMO_COUPLE_MOSI 23
#define THERMO_COUPLE_SLAVE 2
#define HEATER1 16
#define HEATER2 17
float thermocoupleTemp, internalTemp;
bool isProcessing;

int ReadThermocoupleValue();
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
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(THERMO_COUPLE_SLAVE, OUTPUT);
  digitalWrite(THERMO_COUPLE_SLAVE, HIGH);

  SPI.begin(THERMO_COUPLE_SCK, THERMO_COUPLE_MISO, THERMO_COUPLE_MOSI,
            THERMO_COUPLE_SLAVE);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE0);

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

  timerCount = 0;
  while (isProcessing) {
    if (interruptCounter > 0){
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
  if (ReadThermocoupleValue() > 0) {
    M5.Lcd.drawString("error!", 0, 0);
    isProcessing = false;
  } else {
    M5.Lcd.drawString(String(thermocoupleTemp, DEC), 50, 0);
    m5.Lcd.drawLine(20 + timerCount, 240 - 20 - (int)thermocoupleTemp,
                    20 + timerCount, 240 - 20 - (int)thermocoupleTemp,
                    TFT_BLUE);
    timerCount++;
  }
}

void DrawAxis() {
  m5.Lcd.drawLine(20, 20, 20, 220, TFT_WHITE);
  m5.Lcd.drawLine(20, 220, 300, 220, TFT_WHITE);
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
    if ((thermocouple & 0x8000) == 0) {
      thermocoupleTemp = (thermocouple >> 2) * 0.25;
    } else {
      thermocoupleTemp = (0x3fff - (thermocouple >> 2) + 1) * -0.25;
    }
    return 0;
  }
}