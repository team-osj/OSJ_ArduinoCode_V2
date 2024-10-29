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

extern float CH1_Curr_D;
extern float CH2_Curr_D;
extern int json_log_flag1;
extern int json_log_flag2;
extern int json_log_flag1_c;
extern int json_log_flag2_c;
extern int json_log_cnt1;
extern int json_log_cnt2;
extern int json_log_millis1;
extern int json_log_millis2;
extern unsigned int timeSendFlag1;
extern unsigned int timeSendFlag2;
extern bool CH1_CurrStatus;
extern bool CH2_CurrStatus;
extern int CH1_Cnt;
extern int CH2_Cnt;
extern unsigned long previousMillis_end1;
extern unsigned long previousMillis_end2;
extern int m1;
extern int m2;

//================================================= 함수

//------------------------------------------------- 건조기 동작 판단
void Dryer_Status_Judgment(float Amps_TRMS, int cnt, int m, unsigned long previousMillis_end, int ChannelNum)
{
  /*if (ChannelNum == 1 && Amps_TRMS > CH1_Curr_D && dryer_cnt1 == 0){ //전류가 흐르면 millis시작
  dryer_cnt1 = 1;
  dryer_prev_millis1 = millis();
  }
  if (ChannelNum == 1 && Amps_TRMS < CH1_Curr_D && dryer_cnt1 == 1){ //전류가 끊기면 cnt로 시작 취소
  dryer_cnt1 = 0;
  }

  if (ChannelNum == 2 && Amps_TRMS > CH2_Curr_D && dryer_cnt2 == 0){
  dryer_cnt2 = 1;
  dryer_prev_millis2 = millis();
  }
  if (ChannelNum == 2 && Amps_TRMS < CH2_Curr_D && dryer_cnt2 == 1){
  dryer_cnt2 = 0;
  }*/
  if (ChannelNum == 1 && Amps_TRMS < CH1_Curr_D && json_log_flag1) {
    if (json_log_flag1_c == 1) {
      json_log_flag1_c = 0;
      String json_log_cnt1_string = String(json_log_cnt1);
      json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
      json_log1[json_log_cnt1_string]["n"] = "C";
      json_log1[json_log_cnt1_string]["s"] = 0;
      if (json_log_cnt1 % 100 == 0) {
        String json_log_data1 = "";
        serializeJson(json_log1, json_log_data1);
        json_log1.clear();
        SendLog(1, json_log_data1);
      }
      json_log_cnt1++;
    }
  }
  if (ChannelNum == 2 && Amps_TRMS < CH2_Curr_D && json_log_flag2) {
    if (json_log_flag2_c == 1) {
      json_log_flag2_c = 0;
      String json_log_cnt2_string = String(json_log_cnt2);
      json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
      json_log2[json_log_cnt2_string]["n"] = "C";
      json_log2[json_log_cnt2_string]["s"] = 0;
      if (json_log_cnt2 % 100 == 0) {
        String json_log_data2 = "";
        serializeJson(json_log2, json_log_data2);
        json_log2.clear();
        SendLog(2, json_log_data2);
      }
      json_log_cnt2++;
    }
  }
  if (ChannelNum == 1 && Amps_TRMS > CH1_Curr_D)
  {
    if (json_log_flag1) {
      if (json_log_flag1_c == 0) {
        json_log_flag1_c = 1;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "C";
        json_log1[json_log_cnt1_string]["s"] = 1;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }
    }
    if (cnt == 1) // CH1 건조기 동작 시작
    {
      timeSendFlag1 = 1;
      json_log_flag1 = 1;
      json_log_cnt1 = 1;
      json_log_millis1 = millis();
      String local_time = "";
      json_log1["START"]["local_time"] = local_time;

      //dryer_cnt1 = 0;
      CH1_Cnt = 0;
      digitalWrite(PIN_CH1_LED, HIGH);
      CH1_CurrStatus = 0;
      Serial.print("CH");
      Serial.print(ChannelNum);
      Serial.println(" Dryer Started");
      SendStatus(ChannelNum, 0);
    }
    m1 = 1;
  }
  else if (ChannelNum == 2 && Amps_TRMS > CH2_Curr_D)
  {
    if (json_log_flag2) {
      if (json_log_flag2_c == 0) {
        json_log_flag2_c = 1;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "C";
        json_log2[json_log_cnt2_string]["s"] = 1;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }
    }
    if (cnt == 1) // CH2 건조기 동작 시작
    {
      timeSendFlag2 = 1;

      json_log_flag2 = 1;
      json_log_cnt2 = 1;
      json_log_millis2 = millis();

      String local_time = "";
      json_log2["START"]["local_time"] = local_time;

      //dryer_cnt2 = 0;
      CH2_Cnt = 0;
      digitalWrite(PIN_CH2_LED, HIGH);
      CH2_CurrStatus = 0;
      Serial.print("CH");
      Serial.print(ChannelNum);
      Serial.println(" Dryer Started");
      SendStatus(ChannelNum, 0);
    }
    m2 = 1;
  }
  else
  {
    if (previousMillis_end > millis())
      previousMillis_end = millis();
    if (m)
    {
      if (ChannelNum == 1)
        previousMillis_end1 = millis();
      if (ChannelNum == 2)
        previousMillis_end2 = millis();
      if (ChannelNum == 1)
        m1 = 0;
      if (ChannelNum == 2)
        m2 = 0;
    }
    else if (cnt)
      ;
    else if (ChannelNum == 1 && millis() - previousMillis_end >= CH1_EndDelay_D) // CH1 건조기 동작 종료
    {
      timeSendFlag1 = 1;

      json_log_flag1_c = 0;
      json_log_flag1 = 0;


      String local_time = "";
      json_log1["END"]["local_time"] = local_time;

      String json_log_data1 = "";
      serializeJson(json_log1, json_log_data1);
      //Serial.println(json_log_data1);
      json_log1.clear();
      Serial.println("CH1 Dryer Ended");
      SendStatus(1, 1);
      SendLog(1, json_log_data1);
      CH1_Cnt = 1;
      digitalWrite(PIN_CH1_LED, LOW);
      CH1_CurrStatus = 1;
    }
    else if (ChannelNum == 2 && millis() - previousMillis_end >= CH2_EndDelay_D) // CH2 건조기 동작 종료
    {
      timeSendFlag2 = 1;

      json_log_flag2_c = 0;
      json_log_flag2 = 0;


      String local_time = "";
      json_log2["END"]["local_time"] = local_time;

      String json_log_data2 = "";
      serializeJson(json_log2, json_log_data2);
      //Serial.println(json_log_data2);
      json_log2.clear();
      Serial.println("CH2 Dryer Ended");
      SendStatus(2, 1);
      SendLog(2, json_log_data2);
      CH2_Cnt = 1;
      digitalWrite(PIN_CH2_LED, LOW);
      CH2_CurrStatus = 1;
    }
  }
}

//------------------------------------------------- 세탁기 동작 판단
void Status_Judgment(float Amps_TRMS, int WaterSensorData, unsigned int l_hour, int cnt, int m, unsigned long previousMillis_end, int ChannelNum)
{
  if (ChannelNum == 1 && (Amps_TRMS > CH1_Curr_W || WaterSensorData || l_hour > CH1_Flow_W) && se_cnt1 == 0) { //세탁기가 동작하면 millis시작
    se_cnt1 = 1;
    se_prev_millis1 = millis();
  }
  if (ChannelNum == 1 && (Amps_TRMS < CH1_Curr_W && !WaterSensorData && l_hour < CH1_Flow_W) && se_cnt1 == 1) { // 전부 멈추면 시작 취소
    se_cnt1 = 0;
  }

  if (ChannelNum == 2 && (Amps_TRMS > CH2_Curr_W || WaterSensorData || l_hour > CH2_Flow_W) && se_cnt2 == 0) {
    se_cnt2 = 1;
    se_prev_millis2 = millis();
  }
  if (ChannelNum == 2 && (Amps_TRMS < CH2_Curr_W && !WaterSensorData && l_hour < CH2_Flow_W) && se_cnt2 == 1) {
    se_cnt2 = 0;
  }

  if (ChannelNum == 1) {
    if (json_log_flag1) {
      if (Amps_TRMS > CH1_Curr_W && json_log_flag1_c == 0) {
        json_log_flag1_c = 1;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "C";
        json_log1[json_log_cnt1_string]["s"] = 1;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }
      if (Amps_TRMS < CH1_Curr_W && json_log_flag1_c == 1) {
        json_log_flag1_c = 0;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "C";
        json_log1[json_log_cnt1_string]["s"] = 0;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }

      if (l_hour > CH1_Flow_W && json_log_flag1_f == 0) {
        json_log_flag1_f = 1;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "F";
        json_log1[json_log_cnt1_string]["s"] = 1;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }
      if (l_hour < CH1_Flow_W && json_log_flag1_f == 1) {
        json_log_flag1_f = 0;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "F";
        json_log1[json_log_cnt1_string]["s"] = 0;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }

      if (WaterSensorData && json_log_flag1_w == 0) {
        json_log_flag1_w = 1;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "W";
        json_log1[json_log_cnt1_string]["s"] = 1;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }
      if (!WaterSensorData && json_log_flag1_w == 1) {
        json_log_flag1_w = 0;
        String json_log_cnt1_string = String(json_log_cnt1);
        json_log1[json_log_cnt1_string]["t"] = millis() - json_log_millis1;
        json_log1[json_log_cnt1_string]["n"] = "W";
        json_log1[json_log_cnt1_string]["s"] = 0;
        if (json_log_cnt1 % 100 == 0) {
          String json_log_data1 = "";
          serializeJson(json_log1, json_log_data1);
          json_log1.clear();
          SendLog(1, json_log_data1);
        }
        json_log_cnt1++;
      }
    }
  }

  if (ChannelNum == 2) {
    if (json_log_flag2) {
      if (Amps_TRMS > CH2_Curr_W && json_log_flag2_c == 0) {
        json_log_flag2_c = 1;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "C";
        json_log2[json_log_cnt2_string]["s"] = 1;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }
      if (Amps_TRMS < CH2_Curr_W && json_log_flag2_c == 1) {
        json_log_flag2_c = 0;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "C";
        json_log2[json_log_cnt2_string]["s"] = 0;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }

      if (l_hour > CH2_Flow_W && json_log_flag2_f == 0) {
        json_log_flag2_f = 1;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "F";
        json_log2[json_log_cnt2_string]["s"] = 1;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }
      if (l_hour < CH2_Flow_W && json_log_flag2_f == 1) {
        json_log_flag2_f = 0;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "F";
        json_log2[json_log_cnt2_string]["s"] = 0;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }

      if (WaterSensorData && json_log_flag2_w == 0) {
        json_log_flag2_w = 1;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "W";
        json_log2[json_log_cnt2_string]["s"] = 1;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }
      if (!WaterSensorData && json_log_flag2_w == 1) {
        json_log_flag2_w = 0;
        String json_log_cnt2_string = String(json_log_cnt2);
        json_log2[json_log_cnt2_string]["t"] = millis() - json_log_millis2;
        json_log2[json_log_cnt2_string]["n"] = "W";
        json_log2[json_log_cnt2_string]["s"] = 0;
        if (json_log_cnt2 % 100 == 0) {
          String json_log_data2 = "";
          serializeJson(json_log2, json_log_data2);
          json_log2.clear();
          SendLog(2, json_log_data2);
        }
        json_log_cnt2++;
      }
    }
  }

  if (ChannelNum == 1 && millis() - se_prev_millis1 >= 500 && se_cnt1 == 1)
  {
    if (cnt == 1) // CH1 세탁기 동작 시작
    {
      timeSendFlag1 = 1;

      json_log_flag1 = 1;
      json_log_cnt1 = 1;
      json_log_millis1 = millis();

      String local_time = "";
      json_log1["START"]["local_time"] = local_time;

      se_cnt1 = 0;
      CH1_Cnt = 0;
      digitalWrite(PIN_CH1_LED, HIGH);
      CH1_CurrStatus = 0;
      Serial.print("CH");
      Serial.print(ChannelNum);
      Serial.println(" Washer Started");
      SendStatus(ChannelNum, 0);
    }
    m1 = 1;
  }
  if (ChannelNum == 2 && millis() - se_prev_millis2 >= 500 && se_cnt2 == 1)
  {
    if (cnt == 1) // CH2 세탁기 동작 시작
    {
      timeSendFlag2 = 1;

      json_log_flag2 = 1;
      json_log_cnt2 = 1;
      json_log_millis2 = millis();

      String local_time = "";
      json_log2["START"]["local_time"] = local_time;

      se_cnt2 = 0;
      CH2_Cnt = 0;
      digitalWrite(PIN_CH2_LED, HIGH);
      CH2_CurrStatus = 0;
      Serial.print("CH");
      Serial.print(ChannelNum);
      Serial.println(" Washer Started");
      SendStatus(ChannelNum, 0);
    }
    m2 = 1;
  }
  else
  {
    if (previousMillis_end > millis())
      previousMillis_end = millis();
    if (m)
    {
      if (ChannelNum == 1)
        previousMillis_end1 = millis();
      if (ChannelNum == 2)
        previousMillis_end2 = millis();
      if (ChannelNum == 1)
        m1 = 0;
      if (ChannelNum == 2)
        m2 = 0;
    }
    else if (cnt)
      ;
    else if (ChannelNum == 1 && millis() - previousMillis_end >= CH1_EndDelay_W) // CH1 세탁기 동작 종료
    {
      timeSendFlag1 = 1;

      json_log_flag1_c = 0;
      json_log_flag1_f = 0;
      json_log_flag1_w = 0;
      json_log_flag1 = 0;
      String local_time = "";
      json_log1["END"]["local_time"] = local_time;

      String json_log_data1 = "";
      serializeJson(json_log1, json_log_data1);
      //Serial.println(json_log_data1);
      json_log1.clear();
      Serial.println("CH1 Washer Ended");
      SendStatus(1, 1);
      SendLog(1, json_log_data1);
      CH1_Cnt = 1;
      digitalWrite(PIN_CH1_LED, LOW);
      CH1_CurrStatus = 1;
    }
    else if (ChannelNum == 2 && millis() - previousMillis_end >= CH2_EndDelay_W) // CH2 세탁기 동작 종료
    {
      timeSendFlag2 = 1;

      json_log_flag2_c = 0;
      json_log_flag2_f = 0;
      json_log_flag2_w = 0;
      json_log_flag2 = 0;
      String local_time = "";
      json_log2["END"]["local_time"] = local_time;

      String json_log_data2 = "";
      serializeJson(json_log2, json_log_data2);
      //Serial.println(json_log_data2);
      json_log2.clear();
      Serial.println("CH2 Washer Ended");
      SendStatus(2, 1);
      SendLog(2, json_log_data2);
      CH2_Cnt = 1;
      digitalWrite(PIN_CH2_LED, LOW);
      CH2_CurrStatus = 1;
    }
  }
}