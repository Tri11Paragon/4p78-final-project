#include "headers.h"
#define PWM_FREQ 400

void wire1(){
  Wire1.begin(SDA, SCL);
  Wire1.setClock(400000);
}

void wire2(){
  Wire1.begin(D7, D5);
  Wire2.setClock(400000);
}

float angleOffset = 2.5033416018486023;
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
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  
  analogWriteFreq(PWM_FREQ);
}

void initI2C(){
  delay(100);  

  wire2();
  wire1();
}

void setup() {
  initSerial();
  initMotors();
  initPID();
  initI2C();
  initDistance();
  initEncoder();
  
  initWifi(true);
  initServer();
  
  initGyro();  
}

void loop() {
  long start = millis();
  
  if (updateGyro()) { //gyro data
    double angle = ypr[1] * 180 / M_PI;
    if(angle>180) 
      angle -= 180;
    angleInput = angle - angleOffset;
  }
  long gyro = millis();
  
  updateEncoder();
  
  long encoder = millis();
  
  updateDistance();
  
  long distance = millis();
  
  updateServer();
  
  long server = millis();
  
  Speeds speeds = updatePID();
  
  long pid = millis();

  speeds.left = min(90.0f-10, max(-90.0f+10, speeds.left));
  speeds.right = min(90.0f-10, max(-90.0f+10, speeds.right));

  const int MAX_FOR = (int)(0.0010/(1.0/PWM_FREQ)*255);
  const int MAX_REV = (int)(0.0020/(1.0/PWM_FREQ)*255);

  analogWrite(D3, map((int)speeds.left, 90-10, -90+10, MAX_REV, MAX_FOR));
  analogWrite(D4, map((int)speeds.right, 90-10, -90+10, MAX_REV, MAX_FOR));

  long end = millis();
  
  if(end-start>LOOP_INTERVAL_MS){
    Serial.print("Overran ");
    Serial.print(gyro-start);
    Serial.print(" ");
    Serial.print(encoder-gyro);
    Serial.print(" ");
    Serial.print(distance-encoder);
    Serial.print(" ");
    Serial.print(server-distance);
    Serial.print(" ");
    Serial.print(pid-server);
    Serial.print(" ");
    Serial.println(end-pid);
  }else{
    delay(LOOP_INTERVAL_MS-(end-start)); 
  }
}
