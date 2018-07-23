#include "ReflowProfile.h"
#include <Arduino.h>

void ReflowProfile::Initialize() {
  timingDataIndex = 0;
  holdCounter = 0;
  isFinished = false;
  pinMode(HEATER1, OUTPUT);
  pinMode(HEATER2, OUTPUT);
}

void ReflowProfile::Stop() {
  timingDataIndex = 0;
  holdCounter = 0;
  isFinished = true;
  SwitchHeater(0);
}

bool ReflowProfile::IsFinished() { return isFinished; }

bool ReflowProfile::IsIndexChanged() {
  if (isIndexChanged) {
    isIndexChanged = false;
    return true;
  } else {
    return false;
  }
}

void ReflowProfile::SwitchHeater(int state) {
  switch (state) {
    case ALL_OFF:
    case FINISH:
      digitalWrite(HEATER1, LOW);
      digitalWrite(HEATER2, LOW);
      break;
    case UP_ON:
      digitalWrite(HEATER1, HIGH);
      digitalWrite(HEATER2, LOW);
      break;
    case DOWN_ON:
      digitalWrite(HEATER1, HIGH);
      digitalWrite(HEATER2, LOW);
      break;
    case ALL_ON:
      digitalWrite(HEATER1, HIGH);
      digitalWrite(HEATER2, HIGH);
      break;
    default:
      digitalWrite(HEATER1, LOW);
      digitalWrite(HEATER2, LOW);
      break;
  }
}

void ReflowProfile::KeepTemperature(float currentTemp, float targetTemp,
                                    int heaterState) {
  if (currentTemp < targetTemp) {
    SwitchHeater(heaterState);
  } else {
    SwitchHeater(0);
  }
}

void ReflowProfile::ControlRoutine(float currentTemp) {
  if (timingData[timingDataIndex][0] == FINISH) {
    isFinished = true;
    SwitchHeater(0);
    return;
  }

  // hold temp
  if (holdCounter > 0) {
    holdCounter--;
    KeepTemperature(currentTemp, timingData[timingDataIndex][1],
                    timingData[timingDataIndex][0]);
    if (holdCounter == 0) {
      timingDataIndex++;
      isIndexChanged = true;
    }
    return;
  }

  // heatup
  if (currentTemp < timingData[timingDataIndex][1]) {
    Serial.println("Heating up");
    SwitchHeater(timingData[timingDataIndex][0]);
    return;
  }

  // start hold timer as heat up
  if (currentTemp > timingData[timingDataIndex][1]) {
    if (timingData[timingDataIndex][2] == 0) {
      timingDataIndex++;
      isIndexChanged = true;
      return;
    }
    holdCounter = timingData[timingDataIndex][2];
  }
}