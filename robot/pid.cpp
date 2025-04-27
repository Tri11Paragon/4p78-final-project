#include "headers.h"
#include <PID_v1.h>

double angKp=0.0444, angKi=0.6666, angKd=0.001;
double posKp=0.8, posKi=0.2, posKd=1;
double turnKp=0.014, turnKi=0.0333, turnKd=0.0;


double angleInput, angleOutput, angleSetpoint;
PID anglePID(&angleInput, &angleOutput, &angleSetpoint, angKp, angKi, angKd, P_ON_M, REVERSE);

double posInput, posOutput, posSetpoint;
PID posPID(&posInput, &posOutput, &posSetpoint, posKp, posKi, posKd, P_ON_E, DIRECT);

double turnInput, turnOutput, turnSetpoint;
PID turnPID(&turnInput, &turnOutput, &turnSetpoint, turnKp, turnKi, turnKd, P_ON_E, DIRECT);

PID* pids[PID_ARR_COUNT] = {&anglePID, &posPID, &turnPID};

const float MAX_TURN_SPEED = 0.15;

void initPID(){
  angleSetpoint = 0;
  anglePID.SetOutputLimits(-1, 1); // speed forward/backward
  anglePID.SetMode(AUTOMATIC);
  anglePID.SetSampleTime(5);

  posSetpoint = 0;
  posPID.SetOutputLimits(-2, 2); // degrees forward/backward
  posPID.SetMode(AUTOMATIC);
  posPID.SetSampleTime(5);
  posPID.SetControllerDirection(DIRECT);

  turnSetpoint = 0;
  turnPID.SetOutputLimits(-MAX_TURN_SPEED, MAX_TURN_SPEED); // speed forward/backward
  turnPID.SetMode(AUTOMATIC);
  turnPID.SetSampleTime(5);
}


Speeds updatePID(){
  posPID.Compute();
  angleSetpoint = posOutput;
  anglePID.Compute();

  turnSetpoint = 0;
  float maxTurn = max(0.0f, MAX_TURN_SPEED-abs((float)angleOutput)/90);
  turnPID.SetOutputLimits(-maxTurn, maxTurn);
  turnPID.Compute();

  Speeds speeds;
  speeds.left = (float)(angleOutput + turnOutput);
  speeds.right = (float)(-angleOutput + turnOutput);
  if(angleInput>20 || angleInput<-20){
    speeds.left = 0;
    speeds.right = 0;
  }
  return speeds;
}
