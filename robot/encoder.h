

struct Encoder{
  int rotations;
  int currentAngle;  
  int estAngle;
  bool threashhold;

  float position(){
    return rotations + (estAngle / 360.0);
  }
};

Encoder encoder;


void initEncoder(){
    pinMode(A0, INPUT);
    encoder.currentAngle = 0;
    encoder.rotations = 0;
    encoder.estAngle=0;
    encoder.threashhold=false;
}

void updateEncoder(){
  int rawIn = analogRead(A0);
//  static long lastRan = 0;
//  if(50>millis()-lastRan) return;
//  lastRan = millis();

  
  int rotation = map(rawIn, 4, 348, 0, 360);
  int diff = rotation-encoder.currentAngle;
  if(!encoder.threashhold && (abs(diff)>=50)){
    encoder.threashhold=true; 
    if(diff<0){
      encoder.rotations++;
      encoder.estAngle = 0;
    }
    if(diff>0){
      encoder.rotations--;
      encoder.estAngle = 360;
    }
  }
  if(abs(diff)<30){
    encoder.threashhold=false; 
    encoder.estAngle = rotation;
  }
  encoder.currentAngle = rotation;
}
