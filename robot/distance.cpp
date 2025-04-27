#include "headers.h"
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
float distanceReading = 0;

void initDistance(){
  if (!lox.begin(0x29, false, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE)) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  lox.startRangeContinuous();
}


void updateDistance(){
  if (lox.isRangeComplete()) {
    distanceReading = (float)lox.readRange();
  }else{
    distanceReading = 8000;
  }
}
