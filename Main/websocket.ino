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

//================================================================ 변수

extern unsigned long last_ping_millis;
extern bool ping_flag;
extern bool mode_debug;
extern bool CH1_Live;
extern bool CH2_Live;
extern bool CH1_CurrStatus;
extern bool CH2_CurrStatus;
extern bool CH1_Mode;
extern bool CH2_Mode;
extern float CH1_Curr_W;
extern float CH2_Curr_W;
extern unsigned int CH1_Flow_W;
extern unsigned int CH2_Flow_W;
extern float CH1_Curr_D;
extern float CH2_Curr_D;
extern unsigned int CH1_EndDelay_W;
extern unsigned int CH2_EndDelay_W;
extern unsigned int CH1_EndDelay_D;
extern unsigned int CH2_EndDelay_D;
extern String CH1_DeviceNo;
extern String CH2_DeviceNo;
extern float Amps_TRMS1;
extern float Amps_TRMS2;
extern unsigned int l_hour1;
extern unsigned int l_hour2;
extern int WaterSensorData1;
extern int WaterSensorData2;
extern String ap_ssid;

//================================================= 함수

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
        last_ping_millis = millis();
        ping_flag = true;
        if (mode_debug)
        {
          if (CH1_Live == true)
          {
            SendStatus(1, CH1_CurrStatus);
          }
          if (CH2_Live == true)
          {
            SendStatus(2, CH2_CurrStatus);
          }
        }
      }
      break;
    case WStype_TEXT:
      {
        Serial.printf("[WSc] get text: %s\n", payload);
        deserializeJson(doc, payload);
        String title = doc["title"];
        if (title == "GetData")
        {
          StaticJsonDocument<600> MyStatus;
          MyStatus["title"] = "GetData";
          if (mode_debug)
          {
            MyStatus["debug"] = "No";
          }
          else
          {
            MyStatus["debug"] = "Yes";
          }
          if (CH1_Mode)
          {
            MyStatus["ch1_mode"] = "Wash";
          }
          else
          {
            MyStatus["ch1_mode"] = "Dry";
          }
          if (CH2_Mode)
          {
            MyStatus["ch2_mode"] = "Wash";
          }
          else
          {
            MyStatus["ch2_mode"] = "Dry";
          }
          if (CH1_CurrStatus)
          {
            MyStatus["ch1_status"] = "Not Working";
          }
          else
          {
            MyStatus["ch1_status"] = "Working";
          }
          if (CH2_CurrStatus)
          {
            MyStatus["ch2_status"] = "Not Working";
          }
          else
          {
            MyStatus["ch2_status"] = "Working";
          }
          MyStatus["CH1_Curr_W"] = CH1_Curr_W;
          MyStatus["CH2_Curr_W"] = CH2_Curr_W;
          MyStatus["CH1_Flow_W"] = CH1_Flow_W;
          MyStatus["CH2_Flow_W"] = CH2_Flow_W;
          MyStatus["CH1_Curr_D"] = CH1_Curr_D;
          MyStatus["CH2_Curr_D"] = CH2_Curr_D;
          MyStatus["CH1_EndDelay_W"] = CH1_EndDelay_W;
          MyStatus["CH2_EndDelay_W"] = CH2_EndDelay_W;
          MyStatus["CH1_EndDelay_D"] = CH1_EndDelay_D;
          MyStatus["CH2_EndDelay_D"] = CH2_EndDelay_D;
          MyStatus["ch1_deviceno"] = CH1_DeviceNo;
          MyStatus["ch2_deviceno"] = CH2_DeviceNo;
          MyStatus["ch1_current"] = Amps_TRMS1;
          MyStatus["ch2_current"] = Amps_TRMS2;
          MyStatus["ch1_flow"] = l_hour1;
          MyStatus["ch2_flow"] = l_hour2;
          MyStatus["ch1_drain"] = WaterSensorData1;
          MyStatus["ch2_drain"] = WaterSensorData2;
          MyStatus["wifi_ssid"] = ap_ssid;
          MyStatus["wifi_rssi"] = WiFi.RSSI();
          MyStatus["wifi_ip"] = WiFi.localIP().toString();
          MyStatus["mac"] = WiFi.macAddress();
          MyStatus["fw_ver"] = build_date;
          String MyStatus_String;
          serializeJson(MyStatus, MyStatus_String);
          webSocket.sendTXT(MyStatus_String);
        }
      }
      break;
    case WStype_PING:
      last_ping_millis = millis();
      break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}