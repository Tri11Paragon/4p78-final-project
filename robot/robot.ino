

#include "headers.h"

void wire1(){
  Wire1.begin(SDA, SCL);
  Wire1.setClock(400000);
}

void wire2(){
  Wire1.begin(D7, D5);
  Wire2.setClock(400000);
}


#include <Servo.h>
Servo left;
Servo right;

float angleOffset = -2.4241745;
float desiredYaw = 0.0;
float currentYaw = 0.0;

FVec2 desiredPos;

DebugState dbgState;

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
  delay(100);  

  wire2();
  wire1();
}

void setup() {
  initSerial();
  initWifi(false);
  initServer();
  initMotors();
  initPID();
  initI2C();
  initDistance();
  initGyro();  
  initEncoder();
}

void loop() {
  long start = millis();
  
  if (updateGyro()) { //gyro data
    double angle = ypr[1] * 180 / M_PI;
    if(angle>180) 
      angle -= 180;
    angleInput = angle + angleOffset;
  }
  
  updateEncoder();
  currentYaw=odom.angle*180/M_PI;
  
  updateDistance();

  
  Speeds speeds = updatePID();

  speeds.left = min(90.0f-10, max(-90.0f+10, speeds.left));
  speeds.right = min(90.0f-10, max(-90.0f+10, speeds.right));
  left.write(90+(int)speeds.left);
  right.write(90+(int)speeds.right);

  long end = millis();
  
  if(end-start>LOOP_INTERVAL_MS){
    Serial.print("Overran ");
    Serial.println(end-start);
  }else{
    delay(LOOP_INTERVAL_MS-(end-start)); 
  }
}
