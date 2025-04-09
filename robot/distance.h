#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
float distanceReading = 0;

void initDistance(){

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  lox.startRangeContinuous();
}


void updateDistance(){
  if (lox.isRangeComplete()) {
    distanceReading = (float)lox.readRange();
//    if(distanceReading>2000){
//      distanceReading = (float)NAN;
//    }
  }
}
