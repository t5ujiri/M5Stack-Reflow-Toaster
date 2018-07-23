#ifndef ReflowProfile_H

#define ReflowProfile_H
#define ALL_OFF 0
#define UP_ON 1
#define DOWN_ON 2
#define ALL_ON 3
#define FINISH 4

#define HEATER1 16
#define HEATER2 17
#include <Arduino.h>

class ReflowProfile {
 private:
  uint16_t timingDataIndex;
  int16_t holdCounter;
  bool isFinished;
  bool isIndexChanged;
  void SwitchHeater(int state);
  void KeepTemperature(float currentTemp, float targetTemp, int heaterState);

  uint16_t timingData[4][3] = {
      {DOWN_ON, 130, 15},   // 2ON   , temperature130, keep15sec
      {ALL_ON, 230, 0},     // 1&2ON , temperature230, keep0sec
      {DOWN_ON, 225, 100},  // 2ON   , temperature225, keep100sec
      {FINISH, 0, 0}       // 1&2OFF, temperature0  , keep0sec
  };

 public:
  void Stop();
  void Initialize();
  void ControlRoutine(float currentTemp);
  bool IsFinished();
  bool IsIndexChanged();
};
#endif