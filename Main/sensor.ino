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

extern volatile int flow_frequency1; // 유량센서 펄스 측정
extern volatile int flow_frequency2; // 유량센서 펄스 측정

//================================================= 함수

//------------------------------------------------- CH1 유량센서 인터럽트
void IRAM_ATTR flow1()
{
  flow_frequency1++;
}

//------------------------------------------------- CH2 유량센서 인터럽트
void IRAM_ATTR flow2()
{
  flow_frequency2++;
}