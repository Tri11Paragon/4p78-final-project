#include "headers.h"
#include <PID_v1.h>

//double angKp=3.5, angKi=80, angKd=0.042;
//double posKp=20, posKi=0.0, posKd=0.0;
//double turnKp=2, turnKi=0.0, turnKd=0.0;


double angKp=4, angKi=60.0, angKd=0.0958;
double posKp=0.5, posKi=0.0, posKd=0.5;
double turnKp=1.25, turnKi=3.0, turnKd=0.0;


double angleInput, angleOutput, angleSetpoint;
PID anglePID(&angleInput, &angleOutput, &angleSetpoint, angKp, angKi, angKd, P_ON_M, REVERSE);

double posInput, posOutput, posSetpoint;
PID posPID(&posInput, &posOutput, &posSetpoint, posKp, posKi, posKd, P_ON_E, DIRECT);

double turnInput, turnOutput, turnSetpoint;
PID turnPID(&turnInput, &turnOutput, &turnSetpoint, turnKp, turnKi, turnKd, P_ON_E, DIRECT);

PID* pids[PID_ARR_COUNT] = {&anglePID, &posPID, &turnPID};

void initPID(){
  angleSetpoint = 0;
  anglePID.SetOutputLimits(-180, 180);
  anglePID.SetMode(AUTOMATIC);
  anglePID.SetSampleTime(5);

  posSetpoint = 0;
  posPID.SetOutputLimits(-2, 2);
  posPID.SetMode(AUTOMATIC);
  posPID.SetSampleTime(5);
  posPID.SetControllerDirection(DIRECT);

  turnSetpoint = 0;
  turnPID.SetOutputLimits(-15, 15);
  turnPID.SetMode(AUTOMATIC);
  turnPID.SetSampleTime(5);
}


Speeds updatePID(){
  posPID.Compute();
  angleSetpoint = posOutput;
  anglePID.Compute();

  turnSetpoint = 0;
  float maxTurn = max(0.0f, 25.0f-abs((float)angleOutput));
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
