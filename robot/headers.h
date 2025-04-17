
#define LOOP_INTERVAL_MS 20

struct FVec2{
  float x,y;
};
struct FVec3{
  float x,y,z;
};
struct FVec4{
  float x,y,z;
};

extern FVec2 desiredPos;

struct Everything{
  float motorTargetAngle;
  float position;
  FVec3 anglePID;
  FVec3 posPID;
  FVec3 ypr;
  FVec3 euler;
  FVec3 gravity;
  FVec4 q;
  FVec3 aa;
  FVec3 gy;
  FVec3 aaReal;
  FVec3 aaWorld;
};

extern float angleOffset;
extern float desiredYaw;
extern float currentYaw;

struct DebugState{
  int motorTargetAngle;
};

extern DebugState dbgState;

//-------- wire
#include <Wire.h>
#define Wire1 Wire
#define Wire2 Wire

void wire1();
void wire2();

//-------- gyro
#include "MPU6050_6Axis_MotionApps612.h"

extern Quaternion q;           // [w, x, y, z]         quaternion container
extern VectorInt16 aa;         // [x, y, z]            accel sensor measurements
extern VectorInt16 gy;         // [x, y, z]            gyro sensor measurements
extern VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
extern VectorInt16 aaRealLast; // [x, y, z]            gravity-free accel sensor measurements
extern VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
extern VectorFloat gravity;    // [x, y, z]            gravity vector
extern float euler[3];         // [psi, theta, phi]    Euler angle container
extern float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

void initGyro();
bool updateGyro();

//-------- distance
extern float distanceReading;

void initDistance();
void updateDistance();

//-------- encoder

struct EncoderOdom{
  float x;
  float y;
  float angle;  
  
  float left;
  float right;
};


extern EncoderOdom odom;

void zeroOdom();
void initEncoder();
void updateEncoder();

//--------- pid
#include <PID_v1.h>
#define PID_ARR_COUNT 3
extern PID* pids[];

extern double angleInput, angleOutput, angleSetpoint;
extern PID anglePID;

extern double posInput, posOutput, posSetpoint;
extern PID posPID;

extern double turnInput, turnOutput, turnSetpoint;
extern PID turnPID;

struct Speeds{
  float left;
  float right;
};
  
Speeds updatePID();
void initPID();

//--------- webserver

void initWifi(bool host);
void initServer();
void updateServer();
