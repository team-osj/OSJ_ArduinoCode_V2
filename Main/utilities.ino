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

extern String Default_Device_Name;
extern String Device_Name;
extern String ap_ssid;
extern String ap_passwd;
extern String serial_no;
extern String auth_id;
extern String auth_passwd;
extern String CH1_DeviceNo;
extern String CH2_DeviceNo;
extern String RoomNo;
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
extern bool CH1_Live;
extern bool CH2_Live;

//================================================= 함수

//------------------------------------------------- 문자열 쓰기
void putString(const char *key, String value)
{
  Serial.print(key);
  Serial.print(" = ");
  Serial.println(value);
  preferences.putString(key, value);
}

//------------------------------------------------- 설정된 데이터 출력
void SetDefaultVal()
{
  ap_ssid = preferences.getString("ap_ssid", "");
  ap_passwd = preferences.getString("ap_passwd", "");
  serial_no = preferences.getString("serial_no", "0");
  auth_id = preferences.getString("AUTH_ID", "");
  auth_passwd = preferences.getString("AUTH_PASSWD", "");

  CH1_DeviceNo = preferences.getString("CH1_DeviceNo", "1");
  CH2_DeviceNo = preferences.getString("CH2_DeviceNo", "2");

  CH1_Curr_W = preferences.getFloat("CH1_Curr_W", 0.2);
  CH2_Curr_W = preferences.getFloat("CH2_Curr_W", 0.2);
  CH1_Flow_W = preferences.getUInt("CH1_Flow_W", 50);
  CH2_Flow_W = preferences.getUInt("CH2_Flow_W", 50);
  CH1_Curr_D = preferences.getFloat("CH1_Curr_D", 0.5);
  CH2_Curr_D = preferences.getFloat("CH2_Curr_D", 0.5);

  CH1_EndDelay_W = preferences.getUInt("CH1_EndDelay_W", 10);
  CH2_EndDelay_W = preferences.getUInt("CH2_EndDelay_W", 10);
  CH1_EndDelay_D = preferences.getUInt("CH1_EndDelay_D", 10);
  CH2_EndDelay_D = preferences.getUInt("CH2_EndDelay_D", 10);

  CH1_Live = preferences.getBool("CH1_Live", true);
  CH2_Live = preferences.getBool("CH2_Live", true);

  RoomNo = preferences.getString("RoomNo", "0");

  Device_Name = Default_Device_Name+serial_no;
  WiFi.setHostname(Device_Name.c_str());
  Serial.print("My Name Is : ");
  Serial.println(Device_Name);
  Serial.print("CH1 : ");
  Serial.print(CH1_DeviceNo);
  Serial.print(" CH2 : ");
  Serial.println(CH2_DeviceNo);
  if (auth_id == "" || auth_passwd == "")
  {
    Serial.println("NO AUTH CODE!!! YOU NEED TO CONFIG SERVER AUTHENTICATION BY AT+SET_AUTH_ID AND AT+SET_AUTH_PASSWD IN DEBUG MODE!!!");
  }
  if (ap_ssid == "")
  {
    Serial.println("NO WIFI SSID!!! YOU NEED TO CONFIG WIFI BY AT+SETAP_SSID AND AT+SETAP_PASSWD IN DEBUG MODE!!!");
  }
  CH1_EndDelay_W *= 10000;
  CH2_EndDelay_W *= 10000;
  CH1_EndDelay_D *= 1000;
  CH2_EndDelay_D *= 1000;
  Serial.print("CH1_Curr_Wash : ");
  Serial.print(CH1_Curr_W);
  Serial.print(" CH2_Curr_Wash : ");
  Serial.println(CH2_Curr_W);

  Serial.print("CH1_Flow_Wash : ");
  Serial.print(CH1_Flow_W);
  Serial.print(" CH2_Flow_Wash : ");
  Serial.println(CH2_Flow_W);

  Serial.print("CH1_Delay_Wash : ");
  Serial.print(CH1_EndDelay_W);
  Serial.print(" CH2_Delay_Wash : ");
  Serial.println(CH2_EndDelay_W);

  Serial.print("CH1_Curr_Dry : ");
  Serial.print(CH1_Curr_D);
  Serial.print(" CH2_Curr_Dry : ");
  Serial.println(CH2_Curr_D);

  Serial.print("CH1_Delay_Dry : ");
  Serial.print(CH1_EndDelay_D);
  Serial.print(" CH2_Delay_Dry : ");
  Serial.println(CH2_EndDelay_D);

  Serial.print("CH1_Enable : ");
  Serial.print(CH1_Live);
  Serial.print(" CH2_Enable : ");
  Serial.println(CH2_Live);
}

//------------------------------------------------- NETWORK 데이터 출력
void NETWORK_INFO()
{
  Serial.print("Name = ");
  Serial.println(Device_Name);
  Serial.println(WiFiStatusCode(WiFi.status()));
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("RSSI = ");
    Serial.println(WiFi.RSSI());
    String ip = WiFi.localIP().toString();
    Serial.printf("Local IP = % s\r\n", ip.c_str());
  }
  Serial.print("MAC = ");
  Serial.println(WiFi.macAddress());
  Serial.print("SSID = ");
  Serial.println(ap_ssid);
  Serial.print("PASSWORD = ");
  Serial.print(ap_passwd);
}