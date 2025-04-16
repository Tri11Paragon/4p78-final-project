#include "headers.h"

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncMessagePack.h>

#include "page_html.h"

AsyncWebServer server(80);

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


void initServer(){
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
      odom.left, odom.right, odom.x, odom.y, odom.angle,
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
