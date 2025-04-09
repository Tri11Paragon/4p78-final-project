#include <PID_v1.h>

//double angKp=3.5, angKi=80, angKd=0.042;
//double posKp=20, posKi=0.0, posKd=0.0;
//double turnKp=2, turnKi=0.0, turnKd=0.0;


double angKp=4.0, angKi=106.75, angKd=0.0472;
double posKp=0.96, posKi=1.28, posKd=0.5;
double turnKp=1, turnKi=5.0, turnKd=0.17;


double angleInput, angleOutput, angleSetpoint;
PID anglePID(&angleInput, &angleOutput, &angleSetpoint, angKp, angKi, angKd, P_ON_M, REVERSE);

double posInput, posOutput, posSetpoint;
PID posPID(&posInput, &posOutput, &posSetpoint, posKp, posKi, posKd, P_ON_E, DIRECT);

double turnInput, turnOutput, turnSetpoint;
PID turnPID(&turnInput, &turnOutput, &turnSetpoint, turnKp, turnKi, turnKd, P_ON_E, REVERSE);

PID* pids[] = {&anglePID, &posPID, &turnPID};

void initPID(){
  angleSetpoint = 0;
  anglePID.SetOutputLimits(-180, 180);
  anglePID.SetMode(AUTOMATIC);
  anglePID.SetSampleTime(5);

  posSetpoint = 0.5;
  posPID.SetOutputLimits(-1, 1);
  posPID.SetMode(AUTOMATIC);
  posPID.SetSampleTime(5);
  posPID.SetControllerDirection(DIRECT);

  turnSetpoint = 0;
  turnPID.SetOutputLimits(-15, 15);
  turnPID.SetMode(AUTOMATIC);
  turnPID.SetSampleTime(5);
}

struct Speeds{
  float left;
  float right;
};
  
Speeds updatePID(){
  posPID.Compute();
  angleSetpoint = posOutput;
  anglePID.Compute();

  float maxTurn = max(0.0f, 15.0f-abs((float)angleOutput));
  turnPID.SetOutputLimits(-maxTurn, maxTurn);
  turnSetpoint = desiredYaw;
  turnInput = currentYaw;
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
