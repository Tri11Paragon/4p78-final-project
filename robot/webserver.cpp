#include "headers.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
unsigned int localUdpPort = 42069; 


#ifdef DO_WEB_SERVER

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncMessagePack.h>

#include "page_html.h"

AsyncWebServer server(80);


void initWebServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.on("/get_stuff_bin", HTTP_GET, [](AsyncWebServerRequest *request){
    static float arr[4];
    arr[0] = odom.angle;
    arr[1] = distanceReading;
    arr[2] = odom.x;
    arr[3] = odom.y;
    request->send(200, "application/octet-stream", (uint8_t*)(const char*)arr, sizeof(arr));
  });
  server.on("/fuckyou", HTTP_GET, [](AsyncWebServerRequest *request){
    float arr[] = {
      odom.angle,
      distanceReading,
      odom.x,
      odom.y
    };
    request->send(200, "application/text", "hello");
  });
  server.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    zeroOdom();
    request->send(200);
  });
  server.on("/get_stuff", HTTP_GET, [](AsyncWebServerRequest *request){
    char buff[1024];
    int ret = snprintf(buff, sizeof(buff), 
      R"({
        "motorTargetAngle": %f,
        "distanceReading": %f,
        "position": %f,
        "anglePID": {"setpoint": %lf, "input": %lf, "output": %lf},
        "posPID": {"setpoint": %lf, "input": %lf, "output": %lf},
        "turnPID": {"setpoint": %lf, "input": %lf, "output": %lf},
        "odom": {"left": %f, "right": %f, "x": %f, "y": %f, "angle": %f},
        "ypr": {"yaw": %f, "pitch": %f, "roll": %f},
        "euler": {"psi": %f, "theta": %f, "phi": %f},
        "gravity": {"x": %f, "y": %f, "z": %f},
        "q": {"w": %f, "x": %f, "y": %f, "z": %f},
        "aa": {"x": %hd, "y": %hd, "z": %hd},
        "gy": {"x": %hd, "y": %hd, "z": %hd},
        "aaReal": {"x": %hd, "y": %hd, "z": %hd},
        "aaWorld": {"x": %hd, "y": %hd, "z": %hd}
      })",
      (float)dbgState.motorTargetAngle, 
      (float)distanceReading,
      0.0, //encoder.position(),
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
    );
    request->send(200, "application/json", buff);
  });
  server.on("/get_pid", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *bodyData, size_t bodyLen, size_t index, size_t total) {
    StaticJsonDocument<256> json;
    deserializeJson(json, &bodyData[index], bodyLen);
    int idx = json["index"];
    if(idx>3){
      request->send(400);
      return;
    }
    PID& pid = *pids[idx];
    char buff[256];
    int ret = snprintf(buff, sizeof(buff), 
      R"({"kp": %lf, "ki": %lf, "kd": %lf, "direction": %d})",
      pid.GetKp(), pid.GetKi(), pid.GetKd(), pid.GetDirection()
    );
    request->send(200, "application/json", buff);
  });

  server.on("/set_desired_pos", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *bodyData, size_t bodyLen, size_t index, size_t total) {
    StaticJsonDocument<256> json;
    deserializeJson(json, &bodyData[index], bodyLen);
    desiredPos.x = json["x"];
    desiredPos.y = json["y"];
    
    request->send(200);
  });
  
  server.on("/set_pid", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *bodyData, size_t bodyLen, size_t index, size_t total) {
    StaticJsonDocument<256> json;
    deserializeJson(json, &bodyData[index], bodyLen);
    int idx = json["index"];
    if(idx>3){
      request->send(400);
      return;
    }

    PID& pid = *pids[idx];
    if(json.containsKey("kp"))
      pid.SetTunings(json["kp"], json["ki"], json["kd"]);
    if(json.containsKey("direction"))
      pid.SetControllerDirection(json["direction"]);
      
    request->send(200);
  });

   server.on("/set_desired_yaw", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *bodyData, size_t bodyLen, size_t index, size_t total) {
      StaticJsonDocument<256> json;
      deserializeJson(json, &bodyData[index], bodyLen);
      desiredYaw = json["yaw"];
      Serial.print(desiredYaw);
      request->send(200);
   });
  server.begin();
}

#endif 

void initServer(){
  #ifdef DO_WEB_SERVER
  initWebServer()
  #endif 
  
  Udp.begin(localUdpPort);
}

struct ZeroPacket{
  static constexpr uint32_t ID = 0;
};
struct GetDataPacket{
  static constexpr uint32_t ID = 1;
};
struct SetTargetPacket{
  static constexpr uint32_t ID = 2;
  FVec2 pos;
};
struct EverythingPacket{
  static constexpr uint32_t ID = 3;
};
struct GetDataPacketPlus{
  static constexpr uint32_t ID = 4;
};
struct GetPIDPacket{
  static constexpr uint32_t ID = 5;
  uint32_t index;
};
struct SetPIDPacket{
  static constexpr uint32_t ID = 6;
  uint32_t index;
  float kp,ki,kd;
  uint32_t direction;
};

struct Packet{
  uint32_t sequence;
  uint32_t id;
  union{
    ZeroPacket zero;
    GetDataPacket get_data;
    SetTargetPacket set_target;
    EverythingPacket everything;
    GetDataPacketPlus get_data_plus;
    GetPIDPacket get_pid_packet;
    SetPIDPacket set_pid_packet;
  } data;
};

struct DataPacket{
    float yaw;
    float distance;
    FVec2 position;
};


struct DataPacketPlus{
    float yaw;
    float desiredYaw;
    float distance;
    FVec2 position;
    FVec2 targetPosition;
};

void respond_data_packet(){
    DataPacket dp;
    dp.yaw = currentYaw;
    dp.distance = distanceReading;
    dp.position.x = odom.x;
    dp.position.y = odom.y;
    
    Udp.write((const char*)&dp, sizeof(dp));
}

void respond_data_packet_plus(){
    DataPacketPlus dp;
    dp.yaw = currentYaw;
    dp.desiredYaw = desiredYaw;
    dp.distance = distanceReading;
    dp.position.x = odom.x;
    dp.position.y = odom.y;
    dp.targetPosition.x = desiredPos.x;
    dp.targetPosition.y = desiredPos.y;
    
    Udp.write((const char*)&dp, sizeof(dp));
}

void respond_everything_packet(){
    float everything[] = {
       (float)dbgState.motorTargetAngle, 
      (float)distanceReading,
      0.0, //encoder.position(),
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

void set_pid(SetPIDPacket spid){
  if(spid.index>PID_ARR_COUNT){
    return;
  }
  
  PID& pid = *pids[spid.index];
  pid.SetTunings(spid.kp, spid.ki, spid.kd);
  pid.SetControllerDirection(spid.direction);
}

void get_pid(GetPIDPacket gpid){
  if(gpid.index>PID_ARR_COUNT){
    return;
  }
  PID& pid = *pids[gpid.index];
  struct {float kp,ki,kd; uint32_t direction;} pidData = {
    .kp=pid.GetKp(),
    .ki=pid.GetKi(),
    .kd=pid.GetKd(),
    .direction=pid.GetDirection()
  };
  
  Udp.write((const char*)&pidData, sizeof(pidData));
}

bool handleUDP(){
  int size = Udp.parsePacket();

  constexpr size_t buffer_size = 256;
  alignas(alignof(float)) char buffer[buffer_size];
  
  if (size>=8){
    int len = Udp.read(buffer, buffer_size);

    Packet* packet = (Packet*)buffer;

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write((const char*)&packet->sequence, sizeof(packet->sequence));
    Udp.write((const char*)&packet->id, sizeof(packet->id));

    switch(packet->id){
      case ZeroPacket::ID: 
        zeroOdom(); 
        desiredPos.x = 0; 
        desiredPos.y = 0; 
        break;  
      case GetDataPacket::ID: 
        respond_data_packet(); 
        break;  
      case SetTargetPacket::ID: 
        desiredPos.x = packet->data.set_target.pos.x; 
        desiredPos.y = packet->data.set_target.pos.y; 
        break;  
      case EverythingPacket::ID: 
        respond_everything_packet(); 
        break;
      case GetDataPacketPlus::ID: 
        respond_data_packet_plus(); 
        break;   
      case SetPIDPacket::ID:
        set_pid(packet->data.set_pid_packet);
        break;
      case GetPIDPacket::ID:
        get_pid(packet->data.get_pid_packet);
        break;
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
