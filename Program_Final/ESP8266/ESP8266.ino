/****************************************************************
Copyright (C) 2021 CSY && SYQ. All rights reserved.
本项目首先需要安装Blinker库，下载地址：https://diandeng.tech/doc/getting-start-8266
使用到的库有：U8G2:项目》加载库》管理库里面下载
           Arduino_PID:https://github.com/br3ttb/Arduino-PID-Library/
该项目的IO定义见原理图
*****************************************************************/
#define BLINKER_WIFI
#define BLINKER_PRINT Serial
#define BLINKER_MIOT_OUTLET  //定义为智能插座

#ifndef STASSID
#define STASSID "Your WIFI SSID"
#define STAPSK  "Your WIFI PASSWORD."
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DHTPIN 5  //DHT11 DATA_PIN 5
#define DHTTYPE DHT11   // 温湿度传感器为DHT 11
/****************音调定义*******************/
#define NOTE_D0 -1
#define NOTE_D1 294
#define NOTE_D2 330
#define NOTE_D3 350
#define NOTE_D4 393
#define NOTE_D5 441
#define NOTE_D6 495
#define NOTE_D7 556

#define NOTE_DL1 147
#define NOTE_DL2 165
#define NOTE_DL3 175
#define NOTE_DL4 196
#define NOTE_DL5 221
#define NOTE_DL6 248
#define NOTE_DL7 278

#define NOTE_DH1 589
#define NOTE_DH2 661
#define NOTE_DH3 700
#define NOTE_DH4 786
#define NOTE_DH5 882
#define NOTE_DH6 990
#define NOTE_DH7 112
/**************节拍定义****************/
#define WHOLE 1
#define HALF 0.5
#define QUARTER 0.25
#define EIGHTH 0.25
#define SIXTEENTH 0.625
/***************************************/

#include <Arduino.h>
#include <U8g2lib.h>
#include <PID_v1.h>
#include <Blinker.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <DHT.h>

char auth[] = "Your Blinker KEY";
char ssid[] = "Your WIFI SSID";
char pswd[] = "Your WIFI PASSWORD.";

const char* password = STAPSK;
int OTA_Flag = 0;  //OTA模式标志位
bool LightState = false;  //小爱同学开关灯标志位
uint32_t read_time = 0;
float humi_read, temp_read;  //DHT11回调
int length;     /* 这里定义一个变量，后面用来表示共有多少个音符 */
int tonePin = 04; /* 蜂鸣器的pin */
/*****************************************************/
//PID参数设置：初始湿度值65%，Kp,Ki,Kd
double Setpoint = 65, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
/*****************************************************/
//WaterPwr：加湿器功率，USBPwr：USB功率，Line：功率条显示
//WaterPowerTmp：0关闭 1功率百分比显示 2全开
//USBPowerTmp：0关闭 1功率百分比显示
//WaterStartTmp：用于加湿器自定义功率时的启动标志位
//OLEDTmp：开关OLED
//AutoHumiTmp：自动湿度控制标志位
/*****************************************************/
float WaterPwr = 0, WaterPwrLine = 0, USBPwr = 0, USBPwrLine = 0;
char WaterPowerTmp = 1, USBPowerTmp = 1, WaterStartTmp = 0, OLEDTmp = 1, AutoHumiTmp = 0;
/*********************天空之城乐谱***************************/
int tune[] = {
  NOTE_D0, NOTE_D0, NOTE_D0, NOTE_D6, NOTE_D7, NOTE_DH1, NOTE_D7, NOTE_DH1, NOTE_DH3, NOTE_D7, NOTE_D7, NOTE_D7, NOTE_D3, NOTE_D3,
  NOTE_D6, NOTE_D5, NOTE_D6, NOTE_DH1, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D3, NOTE_D4, NOTE_D3, NOTE_D4, NOTE_DH1,
  NOTE_D3, NOTE_D3, NOTE_D0, NOTE_DH1, NOTE_DH1, NOTE_DH1, NOTE_D7, NOTE_D4, NOTE_D4, NOTE_D7, NOTE_D7, NOTE_D7, NOTE_D0, NOTE_D6, NOTE_D7,
  NOTE_DH1, NOTE_D7, NOTE_DH1, NOTE_DH3, NOTE_D7, NOTE_D7, NOTE_D7, NOTE_D3, NOTE_D3, NOTE_D6, NOTE_D5, NOTE_D6, NOTE_DH1,
  NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D2, NOTE_D3, NOTE_D4, NOTE_DH1, NOTE_D7, NOTE_D7, NOTE_DH1, NOTE_DH1, NOTE_DH2, NOTE_DH2, NOTE_DH3, NOTE_DH1, NOTE_DH1, NOTE_DH1,
  NOTE_DH1, NOTE_D7, NOTE_D6, NOTE_D6, NOTE_D7, NOTE_D5, NOTE_D6, NOTE_D6, NOTE_D6, NOTE_DH1, NOTE_DH2, NOTE_DH3, NOTE_DH2, NOTE_DH3, NOTE_DH5,
  NOTE_DH2, NOTE_DH2, NOTE_DH2, NOTE_D5, NOTE_D5, NOTE_DH1, NOTE_D7, NOTE_DH1, NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH3,
  NOTE_D6, NOTE_D7, NOTE_DH1, NOTE_D7, NOTE_DH2, NOTE_DH2, NOTE_DH1, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_DH4, NOTE_DH3, NOTE_DH2, NOTE_DH1,
  NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH6, NOTE_DH6, NOTE_DH5, NOTE_DH5, NOTE_DH3, NOTE_DH2, NOTE_DH1, NOTE_DH1, NOTE_D0, NOTE_DH1,
  NOTE_DH2, NOTE_DH1, NOTE_DH2, NOTE_DH2, NOTE_DH5, NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH3, NOTE_DH6, NOTE_DH6, NOTE_DH5, NOTE_DH5,
  NOTE_DH3, NOTE_DH2, NOTE_DH1, NOTE_DH1, NOTE_D0, NOTE_DH1, NOTE_DH2, NOTE_DH1, NOTE_DH2, NOTE_DH2, NOTE_D7, NOTE_D6, NOTE_D6, NOTE_D6, NOTE_D6, NOTE_D7
};
float duration[] = {
  1, 1, 1, 0.5, 0.5,     1 + 0.5, 0.5, 1, 1,     1, 1, 1, 0.5, 0.5,
  1 + 0.5, 0.5, 1, 1,     1, 1, 1, 1,          1 + 0.5, 0.5, 1, 1,
  1, 1, 0.5, 0.5, 0.5, 0.5,    1 + 0.5, 0.5, 1, 1,     1, 1, 1, 0.5, 0.5,
  1 + 0.5, 0.5, 1, 1,    1, 1, 1, 0.5, 0.5,     1 + 0.5, 0.5, 1, 1,
  1, 1, 1, 0.5, 0.5,    1, 0.5, 0.25, 0.25, 0.25, 0.5,    0.5, 0.5, 0.5, 0.25, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 1, 1,    1, 1, 1, 0.5, 0.5,    1 + 0.5, 0.5, 1, 1,
  1, 1, 1, 0.5, 0.5,    1.5, 0.5, 1, 1,    1, 1, 1, 1,
  0.5, 0.5, 1, 1, 0.5, 0.5,    1.5, 0.25, 0.5, 1,    1, 1, 1, 1,
  1, 1, 1, 1,    1, 1, 1, 1,    0.5, 0.5, 1, 1, 0.5, 0.5,
  1, 0.5, 0.5, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,
  0.5, 0.5, 1, 1, 0.5, 0.5,    1, 0.5, 0.25, 0.5, 1,    1, 1, 1, 0.5, 0.5
};
/*********************天空之城乐谱***************************/

BlinkerButton Button1("ddl1"); //开加湿器
BlinkerButton Button2("ddl2"); //关加湿器
BlinkerButton Button3("ddl3"); //开USB
BlinkerButton Button4("ddl4"); //关USB
BlinkerButton Button5("ddl5"); //OTA升级
BlinkerButton Button6("ddl6"); //蜂鸣器播放音乐
BlinkerButton Button11("ddl7"); //开关省电模式

BlinkerButton Button7("led1");  //底板LED
BlinkerButton Button8("led2");  //底板LED
BlinkerButton Button9("led3");  //底板LED
BlinkerButton Button10("led4");  //底板LED
BlinkerButton Button12("oled");  //OLED开关显示
BlinkerButton Button13("autobt");  //自动湿度模式按钮
BlinkerButton Button14("all_led");  //开光所有氛围灯
BlinkerSlider Slider1("beep");  //蜂鸣器自定义音调
BlinkerSlider Slider2("water");  //加湿器自定义功率
BlinkerSlider Slider3("usbled");  //USB接口自定义功率
BlinkerSlider Slider4("autohumi");  //设置自动湿度数值
BlinkerNumber HUMI("humi");  //显示当前湿度
BlinkerNumber TEMP("temp");  //显示当前温度
BlinkerNumber WATERPWR("waterpwr");  //显示当前加湿器功率
BlinkerNumber USBPWR("usbpwr");  //显示当前USB功率
DHT dht(DHTPIN, DHTTYPE);

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 2, /* data=*/ 0, /* reset=*/ U8X8_PIN_NONE);

//拖动滑动条1，执行该函数--蜂鸣器自定义音调
void beep_callback(int32_t value)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("get slider value: ", value);
  tone(tonePin, value);
}

//拖动滑动条2，执行该函数--加湿器自定义功率
void water_callback(int32_t value)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("get slider value: ", value);
  WaterPwr = value; WaterPwrLine = value;
  WaterPwr = (WaterPwr / 255) * 100;
  WaterPwrLine = (WaterPwrLine / 255) * 40;
  WaterPowerTmp = 1;
  if (WaterStartTmp == 0) {
    WaterStartTmp = 1;
    digitalWrite(12, HIGH);
    Blinker.delay(100);
  }
  if (value == 0)
    WaterStartTmp = 0;
  analogWrite(12, value);
  WATERPWR.print(WaterPwr);

  if (value == 0) {
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, LOW);
  }
  else if (value > 0 && value <= 85 && WaterPowerTmp == 1) {
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, LOW);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  }
  else if (value > 85 && value <= 170 && WaterPowerTmp == 1) {
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, LOW);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  }
  else if (value > 170 && value <= 255 && WaterPowerTmp == 1) {
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, LOW);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  }
}

//拖动滑动条3，执行该函数--USB自定义功率
void usb_callback(int32_t value)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("get slider value: ", value);
  USBPwr = value; USBPwrLine = value;
  USBPwr = (USBPwr / 255) * 100;
  USBPwrLine = (USBPwrLine / 255) * 40;
  USBPowerTmp = 1;
  analogWrite(14, value);
  USBPWR.print(USBPwr);
}

//拖动滑动条4，执行该函数--设置自动湿度数值
void auto_callback(int32_t value)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("get slider value: ", value);
  Setpoint = value;
}

//按键1按下执行该函数--开加湿器
void button1_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  digitalWrite(12, HIGH);
  WaterPowerTmp = 2;
  WaterPwr = 100.00;
  WATERPWR.print(WaterPwr);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, LOW); digitalWrite(13, LOW);
  Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, HIGH);
  Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, HIGH); digitalWrite(13, HIGH);
}

//按键2按下执行该函数--关加湿器
void button2_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  WaterPowerTmp = 0;
  WaterPwr = 00.00;
  WATERPWR.print(WaterPwr);
  digitalWrite(12, LOW);
  digitalWrite(16, HIGH); digitalWrite(13, LOW); Blinker.delay(200);
  digitalWrite(16, LOW); digitalWrite(13, LOW);
}

//按键3按下执行该函数--开灯
void button3_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  USBPowerTmp = 2;
  USBPwr = 100.00;
  USBPWR.print(USBPwr);
  digitalWrite(14, HIGH);
}

//按键4按下执行该函数--关灯
void button4_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  USBPowerTmp = 0;
  USBPwr = 00.00;
  USBPWR.print(USBPwr);
  digitalWrite(14, LOW );
}

//按键5按下执行该函数--空中升级
void button5_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, HIGH);
  tone(tonePin, NOTE_D1); Blinker.delay(230);
  tone(tonePin, NOTE_D2); Blinker.delay(230);
  tone(tonePin, NOTE_D3); Blinker.delay(300);
  digitalWrite(04, LOW);
  Blinker.delay(500);
  OTA_Flag = 1;
}

//按键6按下执行该函数--蜂鸣器播放歌曲
void button6_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  for (int x = 0; x < 21; x++) {
    /* 此函数依次播放tune序列里的数组，即每个音符 */
    tone(tonePin, tune[x]);
    /* 每个音符持续的时间，即节拍duration，是调整时间的越大，曲子速度越慢 */
    Blinker.delay(400 * duration[x]);
    /* 停止当前音符，进入下一音符 */
    noTone(tonePin);
  }
  digitalWrite(04, LOW);
}

//按键7按下执行该函数--开关紫氛围灯
void button7_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);

    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button7.print("on");
  }
  else if (state == "off") {
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button7.print("off");
  }
}

//按键8按下执行该函数--开关蓝氛围灯
void button8_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button8.print("on");
  }
  else if (state == "off") {
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button8.print("off");
  }
}

//按键9按下执行该函数--开关白氛围灯
void button9_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button9.print("on");
  }
  else if (state == "off") {
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button9.print("off");
  }
}

//按键10按下执行该函数--开关红氛围灯
void button10_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button10.print("on");
  }
  else if (state == "off") {
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, HIGH);
    Button10.print("off");
  }
}

//按键11按下执行该函数--是否启用省电模式
void button11_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  char WaterTemp = 0;
  WaterTemp = (WaterPwr / 100) * 255;
  if (state == "on") {  //省电模式
    digitalWrite(15, LOW);
    WaterPowerTmp = 0;
    USBPowerTmp = 0;
    Button11.print("on");
  }
  else if (state == "off") {  //正常模式
    digitalWrite(15, HIGH);
    WaterPowerTmp = 1;
    USBPowerTmp = 1;
    digitalWrite(12, HIGH);
    Blinker.delay(100);
    analogWrite(12,WaterTemp);
    Button11.print("off");
  }
}

//按键12按下执行该函数--是否开启OLED
void button12_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {  //省电模式
    OLEDTmp = 0;
    Button12.print("on");
  }
  else if (state == "off") {  //正常模式
    OLEDTmp = 1;
    Button12.print("off");
  }
}

//按键13按下执行该函数--是否开启PID
void button13_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {  //on
    AutoHumiTmp = 1;
    WaterPowerTmp = 1;
    Button13.print("on");
  }
  else if (state == "off") {  //off
    AutoHumiTmp = 0;
    Button13.print("off");
  }
}

//按键14按下执行该函数--开关所有氛围灯
void button14_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {  //on
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);
    
    digitalWrite(16, LOW); digitalWrite(13, HIGH); Blinker.delay(200);
    digitalWrite(16, HIGH); digitalWrite(13, LOW);Blinker.delay(200);
    Button14.print("on");
  }
  else if (state == "off") {  //off
    digitalWrite(16, LOW); digitalWrite(13, LOW); Blinker.delay(50);
    Button14.print("off");
  }
}

//程序接收小爱同学指令
void miotPowerState(const String & state)
{
  BLINKER_LOG("need set power state: ", state);

  if (state == BLINKER_CMD_ON) {
    digitalWrite(LED_BUILTIN, HIGH);
    WaterPowerTmp = 2;
    digitalWrite(12, HIGH);
    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();
    LightState = true;
  }
  else if (state == BLINKER_CMD_OFF) {
    digitalWrite(LED_BUILTIN, LOW);
    WaterPowerTmp = 0;
    digitalWrite(12, LOW);
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();
    LightState = false;
  }
}

//程序向小爱同学返回开关状态
void miotQuery(int32_t queryCode)
{
  BLINKER_LOG("MIOT Query codes: ", queryCode);

  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
      BLINKER_LOG("MIOT Query All");
      BlinkerMIOT.powerState(LightState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("MIOT Query Power State");
      BlinkerMIOT.powerState(LightState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    default :
      BlinkerMIOT.powerState(LightState ? "on" : "off");
      BlinkerMIOT.print();
      break;
  }
}

//心跳包，30-60S自动向服务器发送一次数值
void heartbeat()
{
  HUMI.print(humi_read);
  TEMP.print(temp_read);
  WATERPWR.print(WaterPwr);
  USBPWR.print(USBPwr);
  Slider4.print(Setpoint);
}

void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);
  Blinker.vibrate();
  uint32_t BlinkerTime = millis();
  Blinker.print(BlinkerTime);
  Blinker.print("millis", BlinkerTime);
}

//使用PID算法自动控制当前湿度
void AutoHumi(int Humi) {
  Input = Humi;
  myPID.Compute();
  WaterPwr = WaterPwrLine = Output;
  WaterPwr = (WaterPwr / 255) * 100;
  WaterPwrLine = (WaterPwrLine / 255) * 40;
  if (Output >= 0 && Output <= 1)
    WaterStartTmp = 0;
  else if (WaterStartTmp == 0) {
    WaterStartTmp = 1;
    digitalWrite(12, HIGH);
    Blinker.delay(100);
  }
  analogWrite(12, Output);
}

//DHT11温湿度传感器读取数值
void DDHHTT11() {
  if (read_time == 0 || (millis() - read_time) >= 2000)
  {
    read_time = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      BLINKER_LOG("Failed to read from DHT sensor!");
      return;
    }

    float hic = dht.computeHeatIndex(t, h, false);

    humi_read = h - 15;
    temp_read = t;

    BLINKER_LOG("Humidity: ", h, " %");
    BLINKER_LOG("Temperature: ", t, " *C");
    BLINKER_LOG("Heat index: ", hic, " *C");
  }
}

/***********************************************/
//
//                  OLED显示屏显示数据
//
/***********************************************/
void OLED() {
  int8_t hour = Blinker.hour();
  int8_t min = Blinker.minute();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
  //u8g2.setFont(u8g2_font_b16_t_japanese1);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
  u8g2.setFontDirection(0);
  u8g2.firstPage();
  do {
    if (WaterPowerTmp == 1) {
      u8g2.setCursor(0, 15);
      u8g2.print("W:");
      u8g2.setCursor(15, 15);
      u8g2.print((int)WaterPwr);
      u8g2.drawVLine(0, 17, 7);
      u8g2.drawHLine(1, 20, (int)WaterPwrLine);
      u8g2.drawVLine((int)WaterPwrLine, 17, 7);
    }
    else if (WaterPowerTmp == 0) {
      u8g2.setCursor(0, 15);
      u8g2.print("W:");  //显示字母W：
      u8g2.setCursor(15, 15);
      u8g2.print("***");  //显示***
    }
    else if (WaterPowerTmp == 2) {
      u8g2.setCursor(0, 15);
      u8g2.print("W:");  //显示字母W：
      u8g2.setCursor(15, 15);
      u8g2.print("USE");  //显示字母USE
    }

    if (USBPowerTmp == 1) {
      u8g2.setCursor(45, 15);
      u8g2.print("U:");
      u8g2.setCursor(60, 15);
      u8g2.print((int)USBPwr);
      u8g2.drawVLine(45, 17, 7);
      u8g2.drawHLine(45, 20, (int)USBPwrLine);
      u8g2.drawVLine(45 + (int)USBPwrLine, 17, 7);
    }
    else if (USBPowerTmp == 0) {
      u8g2.setCursor(45, 15);
      u8g2.print("U:");
      u8g2.setCursor(60, 15);
      u8g2.print("***");
    }
    else if (USBPowerTmp == 2) {
      u8g2.setCursor(45, 15);
      u8g2.print("U:");
      u8g2.setCursor(60, 15);
      u8g2.print("USE");
    }

    if (OLEDTmp == 1) {
      u8g2.setPowerSave(0);
    }
    else {
      u8g2.setPowerSave(1);
    }

    u8g2.setCursor(87, 15);
    u8g2.print(hour);
    u8g2.setCursor(103, 15);
    u8g2.print(":");
    u8g2.setCursor(110, 15);
    u8g2.print(min);
    if (AutoHumiTmp == 0) {
      u8g2.setCursor(0, 40);
      u8g2.print("Temp:      Deg");    // 显示Temp:
      u8g2.setCursor(42, 40);
      u8g2.print(temp_read);    // 显示温度数值

      u8g2.setCursor(0, 60);
      u8g2.print("Humi:      %");    // 显示Humi:
      u8g2.setCursor(42, 60);
      u8g2.print(humi_read);    // 显示湿度数值
    }
    else if (AutoHumiTmp == 1) {
      u8g2.setCursor(90, 40);
      u8g2.print("Auto");    // Auto

      u8g2.setCursor(0, 40);
      u8g2.print("Temp:");    // 显示Temp:
      u8g2.setCursor(42, 40);
      u8g2.print(temp_read);    // 显示温度数值

      u8g2.setCursor(0, 60);
      u8g2.print("SetH:    N:");    // 显示设定的Humi:
      u8g2.setCursor(42, 60);
      u8g2.print((int)Setpoint);    // 显示设定湿度数值
      u8g2.setCursor(87  , 60);
      u8g2.print(humi_read);    // 显示当前湿度数值
    }

  } while ( u8g2.nextPage() );
  Blinker.delay(1000);
}

void setup()
{
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(04, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(04, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);
  digitalWrite(14, LOW);
  digitalWrite(15, HIGH);  //3v3->5v EN
  digitalWrite(16, HIGH);

  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Blinker.attachData(dataRead);
  BlinkerMIOT.attachQuery(miotQuery);
  BlinkerMIOT.attachPowerState(miotPowerState);
  Blinker.attachData(dataRead);
  Blinker.attachHeartbeat(heartbeat);
  Blinker.setTimezone(8.0);
  Button1.attach(button1_callback);
  Button2.attach(button2_callback);
  Button3.attach(button3_callback);
  Button4.attach(button4_callback);
  Button5.attach(button5_callback);
  Button6.attach(button6_callback);
  Button7.attach(button7_callback);
  Button8.attach(button8_callback);
  Button9.attach(button9_callback);
  Button10.attach(button10_callback);
  Button11.attach(button11_callback);
  Button12.attach(button12_callback);
  Button13.attach(button13_callback);
  Button14.attach(button14_callback);
  Slider1.attach(beep_callback);
  Slider2.attach(water_callback);
  Slider3.attach(usb_callback);
  Slider4.attach(auto_callback);
  Slider1.print(0);
  Slider2.print(0);
  Slider3.print(0);
  
  dht.begin();
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
  length = sizeof(tune) / sizeof(tune[0]);
  tone(tonePin, NOTE_D1); Blinker.delay(1500); digitalWrite(04, LOW);
  /*******************OTA***********************/
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    Blinker.delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    tone(tonePin, NOTE_D3); Blinker.delay(230);
    tone(tonePin, NOTE_D2); Blinker.delay(230);
    tone(tonePin, NOTE_D1); Blinker.delay(300);
    digitalWrite(04, LOW);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    tone(tonePin, NOTE_D1); Blinker.delay(1000); digitalWrite(04, LOW); Blinker.delay(300);
    tone(tonePin, NOTE_D1); Blinker.delay(1000); digitalWrite(04, LOW);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*******************PID********************/
  //initialize the variables we're linked to
  //  Input = analogRead(PIN_INPUT);
  //  Setpoint = 100;
  //turn the PID on
  myPID.SetMode(AUTOMATIC);  //PID自动模式（只有自动模式才生效）
}

void loop()
{
  if (OTA_Flag == 0) {  //OTA标志位，=0时正常运行主程序
    Blinker.run();  //点灯科技运行函数
    OLED();  //OLED显示函数
    DDHHTT11();  //温湿度传感器函数
    if (AutoHumiTmp == 1) {  //自动湿度模式标志位，=1时开始自动湿度模式
      AutoHumi((int)humi_read);
    }
  }
  else {  //否则（=1）时运行OTA程序
    ArduinoOTA.handle();
  }
}
