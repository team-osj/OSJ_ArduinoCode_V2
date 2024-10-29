//================================================= 라이브러리

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "EmonLib.h"
#include <ESPAsyncWebSrv.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Update.h>
//------------------------------------------------- html
#include "manager_html.h"
#include "ok_html.h"
//------------------------------------------------- server
#include "ServerInfo.h"
//------------------------------------------------- function
#include "sensor.h"
#include "websocket.h"
#include "setvar.h"
#include "judgment.h"
#include "ota.h"
#include "send.h"
#include "utilities.h"

//================================================= 변수

extern bool CH1_Live;
extern bool CH2_Live;
extern String CH1_DeviceNo;
extern String CH2_DeviceNo;
extern unsigned int timeSendFlag1;
extern unsigned int timeSendFlag2;

//================================================= 함수

//------------------------------------------------- server로 상태 전송
int SendStatus(int ch, bool status)
{
  if (ch == 1 && CH1_Live == false)
    return 1;
  if (ch == 2 && CH2_Live == false)
    return 1;
  if (WiFi.status() == WL_CONNECTED && webSocket.isConnected() == true)
  {
    StaticJsonDocument<100> CurrStatus;
    CurrStatus["title"] = "Update";
    if (ch == 1)
    {
      CurrStatus["id"] = CH1_DeviceNo;
      CurrStatus["type"] = timeSendFlag1;
      timeSendFlag1 = 0;
    }
    if (ch == 2)
    {
      CurrStatus["id"] = CH2_DeviceNo;
      CurrStatus["type"] = timeSendFlag2;
      timeSendFlag2 = 0;
    }
    CurrStatus["state"] = status;
    String CurrStatus_String;
    serializeJson(CurrStatus, CurrStatus_String);
    webSocket.sendTXT(CurrStatus_String);
    return 0;
  }
  else
  {
    Serial.println("SendStatus Fail - No Server Connection");
    return 1;
  }
}

//------------------------------------------------- DeviceNo json 변환 후 전송
int SendLog(int ch, String log)
{
  if (ch == 1 && CH1_Live == false)
    return 1;
  if (ch == 2 && CH2_Live == false)
    return 1;
  if (WiFi.status() == WL_CONNECTED && webSocket.isConnected() == true)
  {
    DynamicJsonDocument LogData(1024);
    LogData["title"] = "Log";
    if (ch == 1)
      LogData["id"] = CH1_DeviceNo;
    if (ch == 2)
      LogData["id"] = CH2_DeviceNo;
    LogData["log"] = log;
    String LogData_String;
    serializeJson(LogData, LogData_String);
    webSocket.sendTXT(LogData_String);
    return 0;
  }
  else
  {
    Serial.println("SendLog Fail - No Server Connection");
    return 1;
  }
}