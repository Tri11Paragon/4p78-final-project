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
  odom.angle += (d_right-d_left)/(WHEEL_DISTANCE);
  float ang = (odom.angle+oldAng)/2;
  currentYaw=odom.angle*180/M_PI;

  float dispx = desiredPos.x-odom.x;
  float dispy = desiredPos.y-odom.y;
  posInput = sqrt(dispx*dispx+dispy*dispy);

  desiredYaw = atan2(dispy, dispx)*180/PI;
  desiredYaw = fmod(desiredYaw+360, 360);
  currentYaw = fmod(fmod(currentYaw, 360)+360, 360);

  double yd = desiredYaw-currentYaw;
  if (yd >= 180) 
    yd -= 360;
  else if (yd <= -180) 
    yd += 360;

  //display
  desiredYaw = currentYaw+yd;
  turnInput = yd;
  if (abs(yd) > 90) {
    turnInput = fmod((yd<0?1:-1)*180+yd, 180);
    posInput = -posInput;
    
    //display
    desiredYaw += 180;
  }
  
  posInput *= cos(turnInput*PI/180);
  
  odom.x += (float)(cos(ang)*displacement);
  odom.y += (float)(sin(ang)*displacement);
}
