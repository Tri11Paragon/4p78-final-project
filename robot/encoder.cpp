#include "headers.h"
#include "AS5600.h"

AS5600 as5600_0(&Wire1);
AS5600 as5600_1(&Wire2);

EncoderOdom odom;

void zeroOdom(){
 
    odom.left = 0.0;
    odom.right = 0.0;
    odom.x = 0.0;
    odom.y = 0.0;
    odom.angle = 0.0; 
    
    wire2();
    as5600_0.resetCumulativePosition(0);
    wire1();
    as5600_1.resetCumulativePosition(0);
}


void initEncoder(){
    odom.left = 0.0;
    odom.right = 0.0;
    odom.x = 0.0;
    odom.y = 0.0;
    odom.angle = 0.0;

    wire2();
    as5600_0.begin();
    Serial.print("Connect device 0: ");
    Serial.println(as5600_0.isConnected() ? "true" : "false");
    as5600_0.resetCumulativePosition(0);
  
    wire1();
    as5600_1.begin();
    Serial.print("Connect device 1: ");
    Serial.println(as5600_1.isConnected() ? "true" : "false");
    as5600_1.resetCumulativePosition(0);
}

void updateEncoder(){
  #define WHEEL_CIRCUM (PI*4.256)
//  #define WHEEL_CIRCUM (1.0)
  #define WHEEL_DISTANCE (8-0.125)

  wire2();
  int rawL = as5600_0.getCumulativePosition();
  wire1();
  int rawR = as5600_1.getCumulativePosition();
  
  float left = rawL/4096.0 * -WHEEL_CIRCUM;
  float right = rawR/4096.0 * WHEEL_CIRCUM;

  float d_left = left-odom.left;
  float d_right= right-odom.right;
  
  odom.left = left;
  odom.right = right;

  float displacement = (d_left + d_right)/2;
  float oldAng = odom.angle;
  odom.angle += (d_left-d_right)/(WHEEL_DISTANCE);
  float ang = (odom.angle+oldAng)/2;

  float dispx = desiredPos.x-odom.x;
  float dispy = desiredPos.y-odom.y;
  posInput = sqrt(dispx*dispx+dispy*dispy);

  desiredYaw = atan2(dispy, dispx)*180/PI;
  if(abs(fmod(currentYaw-desiredYaw, 180.0))>90){
    desiredYaw = fmod(desiredYaw+180.0, 360.0);
    posInput = -posInput;
  }
//  Serial.println(desiredYaw);
  
  odom.x += (float)(cos(ang)*displacement);
  odom.y += (float)(sin(ang)*displacement);
}
