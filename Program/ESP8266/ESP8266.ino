#define BLINKER_WIFI
#define BLINKER_PRINT Serial
#define BLINKER_MIOT_OUTLET

#ifndef STASSID
#define STASSID "DB202"
#define STAPSK  "0102030405"
//#define STASSID "3s417"
//#define STAPSK  "qaz753951*"
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DHTPIN 5
#define DHTTYPE DHT11   // DHT 11
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
#include <Blinker.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <DHT.h>

char auth[] = "43738532e503";
char ssid[] = "DB202";
char pswd[] = "0102030405";
//char ssid[] = "3s417";
//char pswd[] = "qaz753951*";
const char* password = STAPSK;
int OTA_Flag = 0;
bool LightState = false;
uint32_t read_time = 0;
float humi_read, temp_read;  //DHT11回调
int length;     /* 这里定义一个变量，后面用来表示共有多少个音符 */
int tonePin = 04; /* 蜂鸣器的pin */
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

BlinkerButton Button1("ddl1"); //blinker按键键名
BlinkerButton Button2("ddl2"); //blinker按键键名
BlinkerButton Button3("ddl3"); //blinker按键键名
BlinkerButton Button4("ddl4"); //blinker按键键名
BlinkerButton Button5("ddl5"); //blinker按键键名
BlinkerButton Button6("ddl6"); //blinker按键键名
BlinkerButton Button7("led1"); //blinker按键键名
BlinkerButton Button8("led2"); //blinker按键键名
BlinkerButton Button9("led3"); //blinker按键键名
BlinkerButton Button10("led4"); //blinker按键键名
BlinkerSlider Slider1("beep");
BlinkerNumber HUMI("humi");
BlinkerNumber TEMP("temp");
DHT dht(DHTPIN, DHTTYPE);

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 2, /* data=*/ 0, /* reset=*/ U8X8_PIN_NONE); 


void beep_callback(int32_t value)
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    BLINKER_LOG("get slider value: ", value);
    tone(tonePin, value);
}

//按键1按下执行该函数--开加湿器
void button1_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //digitalWrite(12, HIGH);
  digitalWrite(16, HIGH);digitalWrite(13, LOW);Blinker.delay(200);
  digitalWrite(16, LOW);digitalWrite(13, HIGH);
}

//按键2按下执行该函数--关加湿器
void button2_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //digitalWrite(12, LOW);
  digitalWrite(16, HIGH);digitalWrite(13, LOW);Blinker.delay(200);
  digitalWrite(16, LOW);digitalWrite(13, HIGH);
}

//按键3按下执行该函数--开灯
void button3_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //digitalWrite(14, HIGH);
  digitalWrite(16, HIGH);digitalWrite(13, LOW);Blinker.delay(200);
  digitalWrite(16, HIGH);digitalWrite(13, HIGH); 
}

//按键4按下执行该函数--关灯
void button4_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //digitalWrite(14, LOW);
  digitalWrite(16, HIGH);digitalWrite(13, LOW);Blinker.delay(200);
  digitalWrite(16, HIGH);digitalWrite(13, HIGH); 
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
  for (int x = 0; x < length; x++) {
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
  if(state == "on"){
    digitalWrite(16, LOW);digitalWrite(13, LOW);Blinker.delay(50);
    
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH); 
    Button7.print("on"); 
  }
  else if(state == "off"){
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button7.print("off");
  }
}

//按键8按下执行该函数--开关蓝氛围灯
void button8_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if(state == "on"){
    digitalWrite(16, LOW);digitalWrite(13, LOW);Blinker.delay(50);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button8.print("on"); 
  }
  else if(state == "off"){
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button8.print("off");
  }
}

//按键9按下执行该函数--开关白氛围灯
void button9_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if(state == "on"){
    digitalWrite(16, LOW);digitalWrite(13, LOW);Blinker.delay(50);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button9.print("on");
  }
  else if(state == "off"){
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button9.print("off");
  }
}

//按键10按下执行该函数--开关红氛围灯
void button10_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  if(state == "on"){
    digitalWrite(16, LOW);digitalWrite(13, LOW);Blinker.delay(50);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Blinker.delay(200);
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button10.print("on");
  }
    else if(state == "off"){
    digitalWrite(16, LOW);digitalWrite(13, HIGH);Blinker.delay(200);
    digitalWrite(16, HIGH);digitalWrite(13, HIGH);
    Button10.print("off");
  }
}

void miotPowerState(const String & state)
{
  BLINKER_LOG("need set power state: ", state);

  if (state == BLINKER_CMD_ON) {
    digitalWrite(LED_BUILTIN, HIGH);

    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();
    LightState = true;
  }
  else if (state == BLINKER_CMD_OFF) {
    digitalWrite(LED_BUILTIN, LOW);

    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();
    LightState = false;
  }
}

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

void heartbeat()
{
    HUMI.print(humi_read);
    TEMP.print(temp_read);
}

void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);
  Blinker.vibrate();
  uint32_t BlinkerTime = millis();
  Blinker.print(BlinkerTime);
  Blinker.print("millis", BlinkerTime);
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
  digitalWrite(15, HIGH);
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
  Slider1.attach(beep_callback);
  dht.begin();
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function  
  length = sizeof(tune) / sizeof(tune[0]);
  tone(tonePin, NOTE_D1); Blinker.delay(1500); digitalWrite(04, LOW);
  /*********************************************/
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
}

uint8_t m = 24;

void loop()
{
  if (OTA_Flag == 0) {
    Blinker.run();
    int8_t month = Blinker.month();
    int8_t mday = Blinker.mday();
    int8_t hour = Blinker.hour();
    int8_t min = Blinker.minute();
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
    /*********OLED显示*********/
    u8g2.setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
    //u8g2.setFont(u8g2_font_b16_t_japanese1);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
    u8g2.setFontDirection(0);
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 15);
      u8g2.print("21");
      u8g2.setCursor(17, 15);
      u8g2.print("年");
      
      u8g2.setCursor(32, 15);
      u8g2.print(month);
      u8g2.setCursor(47, 15);
      u8g2.print("月");
      
      u8g2.setCursor(62, 15);
      u8g2.print(mday);

      u8g2.setCursor(87, 15);
      u8g2.print(hour);
      u8g2.setCursor(103, 15);
      u8g2.print(":");
      u8g2.setCursor(110, 15);
      u8g2.print(min);
      
      u8g2.setCursor(0, 40);
      u8g2.print("Temp:      Deg");    // 显示Temp:
      u8g2.setCursor(42, 40);
      u8g2.print(temp_read);    // 显示温度数值
      
      u8g2.setCursor(0, 60);
      u8g2.print("Humi:      %");    // 显示Humi:
      u8g2.setCursor(42, 60);
      u8g2.print(humi_read);    // 显示湿度数值
      
      //u8g2.print("こんにちは世界");    // Japanese "Hello World" 
    } while ( u8g2.nextPage() );
    Blinker.delay(1000);
  }
  else {
    ArduinoOTA.handle();
  }
}
