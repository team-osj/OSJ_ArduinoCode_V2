//================================================= 라이브러리

#include <Arduino.h> //아두이노 표준 타입 라이브러리
#include <stdlib.h> //메모리 관리, 문자열 변환 라이브러리
#include <string.h> //문자열 관련 라이브러리
#include <WiFi.h> //WiFi를 사용하기 위한 라이브러리
#include <WiFiClientSecure.h> //WiFi를 사용하기 위한 라이브러리
#include <WebSocketsClient.h> //webSocket을 사용하기 위한 라이브러리
#include <ArduinoJson.h> //JSON 라이브러리
#include <Preferences.h> //Preferences 클래스 사용하기 위한 라이브러리
#include <nvs_flash.h> //모든 환경 변수를 삭제하기 위한 라이브러리
#include "EmonLib.h" //전력 모니터링 라이브러리
#include <ESPAsyncWebSrv.h> //비동기 HTTP & WebSocket 서버 라이브러리
#include <AsyncTCP.h> //비동기 TCP 라이브러리
#include <ESPmDNS.h> //DNS 응답, HTTP 라이브러리
#include <Update.h> //OTA 펌웨어 업데이트 라이브러리
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

//================================================= define

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
//------------------------------------------------- GPIO pin
#define PIN_STATUS 17
#define PIN_CH1_LED 18
#define PIN_CH2_LED 19
#define PIN_CH1_MODE 33
#define PIN_CH2_MODE 32
#define PIN_DEBUG 14
#define PIN_CT1 35
#define PIN_CT2 34
#define PIN_FLOW1 27
#define PIN_FLOW2 26
#define PIN_DRAIN1 23
#define PIN_DRAIN2 25
//------------------------------------------------- network
#define PING_LATE_MILLIS 21500
#define sensPeriod 500
//------------------------------------------------- Set
EnergyMonitor ct1; //전력 모니터링 클래스 생성
EnergyMonitor ct2; //전력 모니터링 클래스 생성
Preferences preferences; //메모리 객체 생성
WebSocketsClient webSocket; //웹 소켓 클라이언트 객체 생성
StaticJsonDocument<200> doc; //JSON 문서 객체 생성
//------------------------------------------------- 비동기 웹 서버 객체 생성
AsyncWebServer server(80);
WiFiClient client;

//================================================= variable

//------------------------------------------------- millis() & if()
bool rebooting = false;
bool ping_flag = false;

unsigned long previousMillis = 0;
unsigned long previousMillis_end1 = 0;
unsigned long previousMillis_end2 = 0;
unsigned long led_millis_prev;
unsigned long curr_millis;
unsigned long server_retry_millis;
unsigned long last_ping_millis = 0;

int m1 = 0, m2 = 0;

unsigned int timeSendFlag1 = 0;
unsigned int timeSendFlag2 = 0;

bool mode_debug = false; //디버그 모드 flag
bool led_status = 0; //LED 상태
//------------------------------------------------- 작동 시작 조건
float CH1_Curr_W;
float CH2_Curr_W;
unsigned int CH1_Flow_W;
unsigned int CH2_Flow_W;
float CH1_Curr_D;
float CH2_Curr_D;
//------------------------------------------------- 작동 종료 조건
unsigned int CH1_EndDelay_W;
unsigned int CH2_EndDelay_W;
unsigned int CH1_EndDelay_D;
unsigned int CH2_EndDelay_D;
//------------------------------------------------- CH1, CH2 상태 & 카운터
bool CH1_Mode = false;
bool CH2_Mode = false;
bool CH1_CurrStatus = 1;
bool CH2_CurrStatus = 1;
int CH1_Cnt = 1;
int CH2_Cnt = 1;
bool CH1_Live = true;
bool CH2_Live = true;
//------------------------------------------------- String Data
String Default_Device_Name = "OSJ_";
String Device_Name;
String ap_ssid;
String ap_passwd;
String serial_no;
String auth_id;
String auth_passwd;
String CH1_DeviceNo;
String CH2_DeviceNo;
String RoomNo = "0";

bool wifi_fail = 1; //WiFi 연결 실패 flag
//------------------------------------------------- 전류
float Amps_TRMS1;
float Amps_TRMS2;
//------------------------------------------------- 배수
int WaterSensorData1 = 0;
int WaterSensorData2 = 0;
//------------------------------------------------- 유량
volatile int flow_frequency1; // 유량센서 펄스 측정
volatile int flow_frequency2; // 유량센서 펄스 측정
unsigned int l_hour1;         // L/hour
unsigned int l_hour2;         // L/hour
//------------------------------------------------- 건조기 관련 변수
/*int dryer_prev_millis1 = 0;
int dryer_prev_millis2 = 0;
int dryer_cnt1 = 0;
int dryer_cnt2 = 0;*/
int json_log_flag1 = 0;
int json_log_flag2 = 0;
int json_log_flag1_c = 0;
int json_log_flag2_c = 0;
int json_log_flag1_f = 0;
int json_log_flag2_f = 0;
int json_log_flag1_w = 0;
int json_log_flag2_w = 0;
int json_log_millis1 = 0;
int json_log_millis2 = 0;
int json_log_cnt1 = 1;
int json_log_cnt2 = 1;
DynamicJsonDocument json_log1(1024);
DynamicJsonDocument json_log2(1024);
//------------------------------------------------- 세탁기 관련 변수
int se_prev_millis1 = 0;
int se_prev_millis2 = 0;
int se_cnt1 = 0;
int se_cnt2 = 0;

//================================================= 함수

//------------------------------------------------- WiFi 연결 성공 시 이벤트 핸들러
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("AP Connected");
}

//------------------------------------------------- IP 주소 할당 시 이벤트 핸들러
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("WiFi connected ");
  Serial.println(WiFi.localIP());
  webSocket.disconnect();
  server_retry_millis = curr_millis;
  String ip = WiFi.localIP().toString();
  // 코드 진행 순서 변경 금지
  webSocket.beginSSL(Server_domain, Server_port, Server_url); //웹 소켓 서버 설정 및 연결 시작
  // 추가 헤더 설정
  char HeaderData[35];
  sprintf(HeaderData, "HWID: %s\r\nCH1: %s\r\nCH2: %s\r\nROOM: %s", serial_no.c_str(), CH1_DeviceNo.c_str(), CH2_DeviceNo.c_str(), RoomNo.c_str());
  webSocket.setExtraHeaders(HeaderData);
  webSocket.setAuthorization(auth_id.c_str(), auth_passwd.c_str());
  //webSocket.enableHeartbeat();
  webSocket.onEvent(webSocketEvent); //웹 소켓 이벤트 핸들러 등록
  MDNS.begin(Device_Name); //mDNS로 로컬 네트워크에서 디바이스를 이름으로 접근 가능하게 설정
  // 코드 진행 순서 변경 금지
  Serial.printf("Host: http://%s.local/\n", Device_Name);
  setupAsyncServer(); //비동기 웹 서버 설정
}

//------------------------------------------------- WiFi 연결 종료 시 이벤트 핸들러
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  WiFi.disconnect(true);
  Serial.print("WiFi Lost. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  wifi_fail = 1;
  WiFi.begin(ap_ssid, ap_passwd);
}

//------------------------------------------------- WiFi 상태 메시지 반환 함수
const char *WiFiStatusCode(wl_status_t status)
{
  switch (status)
  {
    case WL_NO_SHIELD: //WiFi 실드가 없을 시
      return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: //WiFi.begin()이 호출될 시
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: //사용 가능한 SSID가 없을 시
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: //스캔 네트워크가 완료 시
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: //WiFi 네트워크에 연결 시
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED: //모든 시도에서 연결이 실패할 시
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: //연결이 끊어졌을 시
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: //네트워크 연결이 끊어졌을 시
      return "WL_DISCONNECTED";
  }
}


//------------------------------------------------- SETUP
void setup()
{
  preferences.begin("config", false); //비휘발성 메모리 초기화
  Serial.begin(115200); //시리얼 통신 초기화 (baudrate : 115200)
  Serial.print("FW_VER : ");
  Serial.println(build_date);
  //pinMode 설정
  pinMode(PIN_STATUS, OUTPUT);         // STATUS
  pinMode(PIN_CH1_LED, OUTPUT);        // CH1
  pinMode(PIN_CH2_LED, OUTPUT);        // CH2
  pinMode(PIN_CH1_MODE, INPUT_PULLUP); // CH1 Mode
  pinMode(PIN_CH2_MODE, INPUT_PULLUP); // CH2 Mode
  pinMode(PIN_DEBUG, INPUT);           // Debug
  pinMode(PIN_CT1, INPUT);             // CT1
  pinMode(PIN_CT2, INPUT);             // CT2
  pinMode(PIN_FLOW1, INPUT);           // FLOW1
  pinMode(PIN_FLOW2, INPUT);           // FLOW2
  pinMode(PIN_DRAIN1, INPUT);          // DRAIN1
  pinMode(PIN_DRAIN2, INPUT);          // DRAIN2
  //전류 센서 초기화
  ct1.current(PIN_CT1, 30.7);
  ct2.current(PIN_CT2, 30.7);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW1), flow1, FALLING); //유량 측정 인터럽트
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW2), flow2, FALLING); //유량 측정 인터럽트
  SetDefaultVal();
  mode_debug = digitalRead(PIN_DEBUG); //디버그 모드 확인
  CH1_Mode = digitalRead(PIN_CH1_MODE);
  CH2_Mode = digitalRead(PIN_CH2_MODE);
  if (mode_debug == 0)
  {
    Serial.println("YOU ARE IN THE DEBUG MODE !!!");
  }
  Serial.print("CH1_Mode : ");
  Serial.println(CH1_Mode);
  Serial.print("CH2_Mode : ");
  Serial.println(CH2_Mode);
  WiFi.disconnect(true); //WiFi 연결 초기화
  for (int i = 0; i < 30; i++) // 센서 안정화
  {
    ct1.calcIrms(1480);
    ct2.calcIrms(1480);
  }
  Serial.print("Boot Heap : ");
  Serial.println(ESP.getFreeHeap());
  digitalWrite(PIN_STATUS, HIGH);
  if (ap_ssid == "")
  {
    Serial.println("Skip WiFi Setting Due to No SSID");
  }
  else
  {
    Serial.print("Connecting to WiFi .. ");
    Serial.println(ap_ssid);
    if (ap_passwd == "")
    {
      WiFi.begin(ap_ssid);
    }
    else
    {
      WiFi.begin(ap_ssid, ap_passwd); //WiFi 연결 시도
    }

    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    int wifi_timeout = 0;

    while (WiFi.status() != WL_CONNECTED) //WiFi 네트워크에 연결되지 않았을 시
    {
      Serial.print('.');
      digitalWrite(PIN_STATUS, LOW);
      delay(100);
      digitalWrite(PIN_STATUS, HIGH);
      delay(100);
      wifi_timeout++;
      if (wifi_timeout > 25) //WiFi 연결 시도 TIME OUT 상태
      {
        Serial.println("Skip WiFi Connection Due to Timeout");
        break;
      }
    }
  }
}
//------------------------------------------------- LOOP
void loop()
{ 
  curr_millis = millis(); //현재 시간 업데이트
  if(ping_flag == true && webSocket.isConnected() == true && curr_millis - last_ping_millis >= PING_LATE_MILLIS){
    ping_flag = false;
    webSocket.disconnect();
  }
  if(rebooting)
  {
    delay(100);
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED) //WiFi 네트워크 연결이 되었을 시
  {
    webSocket.loop(); //웹소켓 통신 처리
    digitalWrite(PIN_STATUS, HIGH);
    wifi_fail = 0;
  }
  else //WiFi 네트워크 연결이 안되었을 시
  {
    wifi_fail = 1;
    if (curr_millis - led_millis_prev >= 100)
    {
      led_millis_prev = curr_millis;
      digitalWrite(PIN_STATUS, !digitalRead(PIN_STATUS)); //LED 깜빡임으로 연결 상태 표시
    }
  }

  //디버그 모드에 따라 다르게 동작
  //------------------------------------------------- 실행 모드
  if (mode_debug)
  {
    Amps_TRMS1 = ct1.calcIrms(1480);
    Amps_TRMS2 = ct2.calcIrms(1480);

    if (previousMillis > millis())
      previousMillis = millis();

    if (millis() - previousMillis >= sensPeriod)
    {
      previousMillis = millis();

      WaterSensorData1 = digitalRead(PIN_DRAIN1);
      WaterSensorData2 = digitalRead(PIN_DRAIN2);

      l_hour1 = (flow_frequency1 * 60/7.5); // L/hour계산
      l_hour2 = (flow_frequency2 * 60/7.5);

      flow_frequency1 = 0; // 변수 초기화
      flow_frequency2 = 0;
    }

    if (CH1_Mode)
    {
      Status_Judgment(Amps_TRMS1, WaterSensorData1, l_hour1, CH1_Cnt, m1, previousMillis_end1, 1);
    }
    else
    {
      Dryer_Status_Judgment(Amps_TRMS1, CH1_Cnt, m1, previousMillis_end1, 1);
    }

    if (CH2_Mode)
    {
      Status_Judgment(Amps_TRMS2, WaterSensorData2, l_hour2, CH2_Cnt, m2, previousMillis_end2, 2);
    }
    else
    {
      Dryer_Status_Judgment(Amps_TRMS2, CH2_Cnt, m2, previousMillis_end2, 2);
    }
  }
  //------------------------------------------------- 디버그 모드
  else
  {
    curr_millis = millis();
    //------------------------------------------------- 상태 표시 LED
    if (curr_millis - led_millis_prev >= 100)
    {
      led_millis_prev = curr_millis;
      if (led_status == 1)
      {
        led_status = 0;
        digitalWrite(PIN_CH1_LED, HIGH);
        digitalWrite(PIN_CH2_LED, LOW);
      }
      else if (led_status == 0)
      {
        led_status = 1;
        digitalWrite(PIN_CH1_LED, LOW);
        digitalWrite(PIN_CH2_LED, HIGH);
      }
    }
    //------------------------------------------------- AT 명령어 입력
    if (Serial.available())
    {
      int dex, dex1, dexc, end;
      String SerialData = Serial.readStringUntil('\n');
      dex = SerialData.indexOf('+');
      dex1 = SerialData.indexOf('"');
      end = SerialData.length();
      String AT_Command = SerialData.substring(dex+1, dex1);
      if (!(AT_Command.compareTo("HELP")))
      {
      }
      else if (!(AT_Command.compareTo("SENSDATA_START")))
      {
        Serial.println("AT+OK SENSDATA_START");
      }
      else if (!(AT_Command.compareTo("SOCKET_SEND")))
      {
      }
      else if (!(AT_Command.compareTo("UPDATE")))
      {
        Serial.println("AT+OK UPDATE");
      }
      else if (!(AT_Command.compareTo("CH1_SETVAR"))) //CH1 데이터 설정
      {
        Serial.println("AT+OK CH1_SETVAR");
        CH1_SETVAR(SerialData, dex1, dexc, end);
      }
      else if (!(AT_Command.compareTo("CH2_SETVAR"))) //CH2 데이터 설정
      {
        Serial.println("AT+OK CH2_SETVAR");
        CH2_SETVAR(SerialData, dex1, dexc, end);
      }
      else if (!(AT_Command.compareTo("UPDATE"))) 
      {
        Serial.println("AT+OK UPDATE");
      }
      else if (!(AT_Command.compareTo("NETWORK_INFO"))) //네트워크 정보 출력
      {
        Serial.println("AT+OK NETWORK_INFO");
        NETWORK_INFO();
      }
      else if (!(AT_Command.compareTo("SETAP_SSID"))) //AP_SSID 변경
      {
        Serial.println("AT+OK SETAP_SSID");
        putString("ap_ssid", SerialData.substring(dex1+1, end - 1));
      }
      else if (!(AT_Command.compareTo("SETAP_PASSWD")))  //AP_PW 변경
      {
        Serial.println("AT+OK SETAP_PASSWD");
        putString("ap_passwd", SerialData.substring(dex1+1, end - 1));
      }
      else if (!(AT_Command.compareTo("SET_SERIALNO"))) //Serial_NUMBER 변경
      {
        Serial.println("AT+OK SET_SERIALNO");
        putString("serial_no", SerialData.substring(dex1+1, end - 1));
      }
      else if (!(AT_Command.compareTo("SET_AUTH_ID"))) //AUTH_ID 변경
      {
        Serial.println("AT+OK SET_AUTH_ID");
        putString("AUTH_ID", SerialData.substring(dex1+1, end - 1));
      }
      else if (!(AT_Command.compareTo("SET_AUTH_PASSWD"))) //AUTH_PW 변경
      {
        Serial.println("AT+OK SET_AUTH_PASSWD");
        putString("AUTH_PASSWD", SerialData.substring(dex1+1, end - 1));
      }
      else if (!(AT_Command.compareTo("FORMAT_NVS"))) //NVS 메모리 포멧
      {
        Serial.println("AT+OK FORMAT_NVS");
        nvs_flash_erase();
        nvs_flash_init();
        ESP.restart();
      }
      else if (!(AT_Command.compareTo("SHOWMETHEMONEY"))) //남은 Heap 출력
      {
        Serial.println("AT+OK SHOWMETHEMONEY");
        Serial.print(ESP.getFreeHeap());
        Serial.println("Byte");
      }
      else if (!(AT_Command.compareTo("WHATTIMEISIT")))
      {
        Serial.println("AT+OK WHATTIMEISIT");
      }
      else if (!(AT_Command.compareTo("REBOOT"))) //재부팅
      {
        Serial.println("AT+OK REBOOT");
        delay(500);
        ESP.restart();
      }
      else //AT 명령어가 일치하지 않을 시
      {
        Serial.println("ERROR: Unknown command");
      }
    }
  }
}