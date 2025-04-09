
#include "I2Cdev.h"


#include <Servo.h>
Servo left;
Servo right;

float angleOffset = -2.4241745;
float desiredYaw = 0.0;
float currentYaw = 0.0;

struct DebugState{
  int motorTargetAngle;
};

DebugState dbgState;

#include "distance.h"
#include "encoder.h"
#include "gyro.h"
#include "pid.h"
#include "webserver.h"


void initSerial(){
  Serial.begin(115200);
  while (!Serial);
  delay(300);
  Serial.print("\n\n\n\n\n\n\n\n\n\n\n");
}

void initMotors(){
  left.attach(D3);
  right.attach(D4);
}

void initI2C(){
  digitalWrite(D2, LOW);
  digitalWrite(D1, LOW);
  delay(100);
  Wire.begin();
  Wire.setClock(400000);
}

void setup() {
  initSerial();
  initWifi(false);
  initServer();
  initMotors();
  initPID();
  initI2C();
  initEncoder();
  initDistance();
  initGyro();
}

void loop() {
  if (updateGyro()) { //gyro data
    currentYaw=ypr[0]*180/M_PI;
    double angle = ypr[1] * 180 / M_PI;
    if(angle>180) 
      angle -= 180;
    angleInput = angle + angleOffset;
  }
  {// encoder data
    updateEncoder();
    posInput = encoder.position();
  }
  updateDistance();

  
  Speeds speeds = updatePID();

  speeds.left = min(90.0f-10, max(-90.0f+10, speeds.left));
  speeds.right = min(90.0f-10, max(-90.0f+10, speeds.right));
  left.write(90+(int)speeds.left);
  right.write(90+(int)speeds.right);
  
  delay(5);
}
