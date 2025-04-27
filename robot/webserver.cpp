#include "headers.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
unsigned int localUdpPort = 42069; 

void initServer(){
  Udp.begin(localUdpPort);
}

// ---------------- response packets
struct DataPacketResponse{
    float yaw;
    float distance;
    FVec2 position;
};


struct DataPlusPacketResponse{
    float yaw;
    float desiredYaw;
    float distance;
    FVec2 position;
    FVec2 targetPosition;
};
// ---------------- response packets

struct ZeroPacket{
  static constexpr uint32_t ID = 0;

  void handle(){
    Udp.write((const char*)&ID, sizeof(ID));
    zeroOdom(); 
    desiredPos.x = 0; 
    desiredPos.y = 0; 
  }
};
struct GetDataPacket{
  static constexpr uint32_t ID = 1;
  
  void handle(){
    Udp.write((const char*)&ID, sizeof(ID));

    DataPacketResponse dp;
    dp.yaw = currentYaw;
    dp.distance = distanceReading;
    dp.position.x = odom.x;
    dp.position.y = odom.y;
    
    Udp.write((const char*)&dp, sizeof(dp));
  }
};
struct SetTargetPacket{
  static constexpr uint32_t ID = 2;
  FVec2 pos;
  
  void handle(){
    desiredPos.x = this->pos.x; 
    desiredPos.y = this->pos.y; 
        
    Udp.write((const char*)&ID, sizeof(ID));
  }
};
struct EverythingPacket{
  static constexpr uint32_t ID = 3;

  void handle(){
    Udp.write((const char*)&ID, sizeof(ID));

    float everything[] = {
      (float)distanceReading,
      angleSetpoint, angleInput, angleOutput,
      posSetpoint, posInput, posOutput,
      turnSetpoint, turnInput, turnOutput,
      odom.left, odom.right, odom.x, odom.y, odom.angle*180/M_PI,
      ypr[0]*180/M_PI, ypr[1]*180/M_PI, ypr[2]*180/M_PI, 
      euler[0]*180/M_PI, euler[1]*180/M_PI, euler[2]*180/M_PI, 
      gravity.x, gravity.y, gravity.z,
      q.w, q.x, q.y, q.z,
      aa.x, aa.y, aa.z,
      gy.x, gy.y, gy.z,
      aaReal.x, aaReal.y, aaReal.z,
      aaWorld.x, aaWorld.y, aaWorld.z
    };
    
    Udp.write((const char*)everything, sizeof(everything));
  }
};
struct GetDataPlusPacket{
  static constexpr uint32_t ID = 4;

  void handle(){
    Udp.write((const char*)&ID, sizeof(ID));

    DataPlusPacketResponse dp;
    dp.yaw = currentYaw;
    dp.desiredYaw = desiredYaw;
    dp.distance = distanceReading;
    dp.position.x = odom.x;
    dp.position.y = odom.y;
    dp.targetPosition.x = desiredPos.x;
    dp.targetPosition.y = desiredPos.y;
    
    Udp.write((const char*)&dp, sizeof(dp));
  }
};
struct GetPIDPacket{
  static constexpr uint32_t ID = 5;
  uint32_t index;

  void handle(){
    if(this->index>PID_ARR_COUNT){
      uint32_t e = -1;
      Udp.write((const char*)&e, sizeof(e));
      return;
    }
    
    Udp.write((const char*)&ID, sizeof(ID));
    PID& pid = *pids[this->index];
    struct {float kp,ki,kd; uint32_t direction;} pidData = {
      .kp=pid.GetKp(),
      .ki=pid.GetKi(),
      .kd=pid.GetKd(),
      .direction=pid.GetDirection()
    };
    
    Udp.write((const char*)&pidData, sizeof(pidData));
  }
};
struct SetPIDPacket{
  static constexpr uint32_t ID = 6;
  uint32_t index;
  float kp,ki,kd;
  uint32_t direction;

  void handle(){
    if(this->index>PID_ARR_COUNT){
      uint32_t e = -1;
      Udp.write((const char*)&e, sizeof(e));
      return;
    }
    Udp.write((const char*)&ID, sizeof(ID));
    
    PID& pid = *pids[this->index];
    pid.SetTunings(this->kp, this->ki, this->kd);
    pid.SetControllerDirection(this->direction);
  }
};

struct Packet{
  uint32_t sequence;
  uint32_t id;
  union{
    ZeroPacket zero;
    GetDataPacket get_data;
    SetTargetPacket set_target;
    EverythingPacket everything;
    GetDataPlusPacket get_data_plus;
    GetPIDPacket get_pid_packet;
    SetPIDPacket set_pid_packet;
  } kind;
};

bool handleUDP(){
  int size = Udp.parsePacket();

  constexpr size_t buffer_size = 256;
  alignas(alignof(float)) char buffer[buffer_size];
  
  if (size>=8){
    int len = Udp.read(buffer, buffer_size);

    Packet* packet = (Packet*)buffer;

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write((const char*)&packet->sequence, sizeof(packet->sequence));

    switch(packet->id){
      case ZeroPacket::ID: packet->kind.zero.handle();break;  
      case GetDataPacket::ID: packet->kind.get_data.handle();break;
      case SetTargetPacket::ID: packet->kind.set_target.handle();break;
      case EverythingPacket::ID: packet->kind.everything.handle();break;
      case GetDataPlusPacket::ID: packet->kind.get_data_plus.handle();break;
      case SetPIDPacket::ID: packet->kind.set_pid_packet.handle();break;
      case GetPIDPacket::ID: packet->kind.get_pid_packet.handle();break;
    }
    Udp.endPacket();
    return true;
  }
  return false;
}

void updateServer(){
  while(handleUDP());
}

void initWifi(bool host){
  if(host){
    WiFi.softAP("MEOW");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }else{
    Serial.print("Connecting to ");
    Serial.println("Michael Loves CP");
    WiFi.enableInsecureWEP(false);
    WiFi.mode(WIFI_STA);
    WiFi.setPhyMode(WIFI_PHY_MODE_11G);
    WiFi.begin("Michael Loves CP", "cockandpavly20");
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print('.');  
    }
    Serial.println();
  }
  Serial.println(WiFi.localIP());
}
