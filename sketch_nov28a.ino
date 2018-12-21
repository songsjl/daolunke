/*
作者: 王漪涵、宋世杰、潘昊宇、徐永华
温度传感器、声音传感器数据上传至onenet，微信小程序报警、硬件端警示牌显示分贝数及校徽、微信小程序实时天气查询
*/
#include <Microduino_Tem_Hum.h>
#include <U8glib.h>    
#define mic_pin A0
#define INTERVAL_LCD 20
Tem_Hum_S2  termo;                                  //调用Sensor-Tem&Hum-S2传感器
unsigned long lcd_time = millis();                  //OLED刷新时间计时器
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);        //设置OLED型号 
//——-字体设置，大、中、小
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_fixed_v0r)
#define setFont_SS u8g.setFont(u8g_font_fub25n)
int RECV_PIN = 10;
void U8GLIB::drawLine(u8g_uint_t x1, u8g_uint_t y1, u8g_uint_t x2, u8g_uint_t y2);
void U8GLIB::drawPixel(uint8_t x, uint8_t y);
void U8GLIB::drawHLine(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w);
//void U8GLIB::drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void U8GLIB::drawFrame(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h);
void U8GLIB::drawRFrame(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, u8g_uint_t r);
void U8GLIB::drawBox(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h);
void U8GLIB::drawRBox(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, u8g_uint_t r);
void U8GLIB::drawCircle(u8g_uint_t x0, u8g_uint_t y0, u8g_uint_t rad, uint8_t opt = U8G_DRAW_ALL);
void U8GLIB::drawXBMP(u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, const u8g_pgm_uint8_t *bitmap);
#define SSID       "iPhone (3)"/* "vivo Y85A"*/ //改为你的Wi-Fi名称
#define PASSWORD    "xy1234567890"//Wi-Fi密码
#define HOST_NAME   "api.heclouds.com"
#define DEVICEID    "502967171" //OneNet上的设备ID
#define PROJECTID   "183352" //OneNet上的产品ID
#define HOST_PORT   (80)
String apiKey="cloAWE7COFXaowclI3TnIGB2q9s=";//与你的设备绑定的APIKey

#define INTERVAL_SENSOR   115             //定义传感器采样时间间隔 
#define INTERVAL_NET      115             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include <ESP8266.h>
#include <I2Cdev.h>                                //调用库  
#include <Microduino_SHT2x.h>
#define  sensorPin_1  A0
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
char buf[7];                               
#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();

float tempOLED, humiOLED, lightnessOLED;

#define INTERVAL_OLED 1000

String mCottenData;
String jsonToSend;

//3,传感器值的设置 
float sensor_tem;                    //传感器温度  
char  sensor_tem_c[7];    //换成char数组传输
char  mic_pin_c[7];
#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(&EspSerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

Tem_Hum_S2 TempMonitor;
void setup() {
  //初始化串口波特率 ,wifi连接 
    Wire.begin();
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.print(F("setup begin\r\n"));
    delay(100);
    pinMode(sensorPin_1, INPUT);

  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("to station + softap ok\r\n"));
  } else {
    Serial.print(F("to station + softap err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));

    Serial.print(F("IP:"));
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }

  Serial.print(F("setup end\r\n"));
    
  
//初始化温湿度传感器，并串口打印传感器是否在线
  pinMode(mic_pin, INPUT);
  termo.begin();
  // Serial.print(termo.begin());                   
  Serial.begin(115200);
}
//OLED显示
//北邮校徽
static unsigned char u8g_go4_bits[] U8G_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0xe0,0x6d,0x87,0x07,0x00,0x00,0x00,0x00,0x98,0x45,0xc1,0x1e,0x00,0x00,0x00,0x00,0xbe,0x4d,0xc6,0x6e,0x00,0x00,0x00,0x80,0xa9,0x52,0xe4,0xf7,0x01,0x00,0x00,0xe0,0xb1,0x56,0x67,0x37,0x03,0x00,0x00,0xb0,0x10,0xfb,0x9f,0x2b,0x06,0x00,0x00,0x78,0x23,0x03,0xe0,0x3a,0x1a,0x00,0x00,0x54,0x61,0x00,0x00,0x1d,0x39,0x00,0x00,0xa2,0x12,0xfe,0x7f,0x88,0x7c,0x00,0x00,0x43,0xc5,0xff,0xff,0xa1,0xc6,0x00,0x80,0x85,0xf3,0xff,0xff,0x47,0xa3,0x00,0xc0,0x06,0xf8,0xff,0xff,0x1f,0x1b,0x01,0xc0,0x58,0xfc,0xff,0xff,0x3f,0x8a,0x03,0x20,0x31,0xfe,0xff,0xff,0x7f,0x4c,0x06,0x70,0x06,0xf0,0xff,0xff,0xff,0x78,0x04,0xd0,0x18,0xc0,0xff,0xff,0xff,0x19,0x0f,0x38,0x0b,0x80,0xff,0xff,0xf7,0x93,0x0d,0x98,0x05,0x00,0xfe,0xff,0xe7,0x47,0x10,0x68,0x06,0x00,0xfc,0xff,0xcf,0xe7,0x1e,0x1c,0x03,0x00,0xfc,0xff,0x9f,0x8f,0x19,0x3c,0x02,0x00,0xf8,0xff,0xbc,0xcf,0x27,0xfc,0x01,0x00,0xf8,0xff,0x7d,0x1f,0x3d,0xc2,0x00,0x00,0xf0,0xff,0x7b,0x9f,0x3f,0x06,0x01,0x00,0xf0,0xbf,0xf7,0x9e,0x5e,0x9e,0x01,0x00,0xf0,0x7f,0x77,0x3e,0x79,0xe6,0x00,0x00,0xf0,0xff,0x0e,0x0c,0x4f,0x7a,0x00,0x00,0xf8,0xff,0x06,0x00,0x7d,0xfe,0x00,0x00,0xf8,0xff,0x00,0x30,0x43,0x9f,0xf0,0x03,0xfe,0xff,0x01,0x3c,0x7f,0x61,0xfc,0xff,0xff,0xff,0x01,0x3c,0x7f,0xff,0xfe,0xff,0xff,0xff,0x03,0x3c,0x43,0x9d,0xff,0xff,0xff,0xff,0x03,0x3c,0x7d,0xfe,0x00,0x00,0x00,0x00,0x00,0x3c,0x7f,0xfe,0x00,0x00,0x00,0x00,0x00,0x3c,0x63,0xc2,0x00,0x00,0x00,0x00,0x00,0x3e,0x7f,0x3e,0x01,0x00,0x00,0x0c,0x00,0x3e,0x5f,0x02,0x01,0x00,0x00,0x0c,0x00,0x9e,0x61,0xe2,0x01,0x00,0x00,0x10,0x00,0x9f,0x31,0xac,0x00,0x00,0x00,0x38,0x00,0x4f,0x26,0x7c,0x02,0x00,0x00,0x10,0x80,0xcf,0x3f,0xc4,0x01,0x00,0x00,0x60,0xc0,0x4f,0x10,0xf8,0x07,0x00,0x00,0x60,0xc0,0xa7,0x11,0xe8,0x06,0x00,0x00,0x00,0xe0,0x63,0x1e,0xb0,0x09,0x00,0x00,0xc0,0xf1,0xb3,0x0d,0xd0,0x1e,0x18,0x00,0xc0,0xfd,0xd9,0x0e,0xa0,0x19,0x7c,0x00,0x80,0xff,0x38,0x05,0xe0,0x3c,0xfe,0x01,0xc0,0x7f,0xf4,0x07,0x40,0xe2,0xfc,0x1f,0xf8,0x3f,0x7e,0x03,0x80,0xd9,0xf8,0xff,0xff,0x8f,0xd7,0x01,0x80,0xa9,0xe2,0xff,0xff,0x47,0xba,0x00,0x00,0xf7,0x84,0xff,0xff,0x20,0xd4,0x00,0x00,0x7a,0x18,0xfc,0x1f,0x18,0x78,0x00,0x00,0x3c,0xf0,0x00,0x00,0x3a,0x30,0x00,0x00,0x18,0x38,0x07,0xe0,0x3c,0x18,0x00,0x00,0x30,0x38,0xf3,0x87,0x7f,0x04,0x00,0x00,0xc0,0x9c,0xf7,0xde,0x39,0x03,0x00,0x00,0x80,0x89,0xf5,0x9e,0xe1,0x01,0x00,0x00,0x00,0x9e,0xf3,0x96,0x72,0x00,0x00,0x00,0x00,0xb8,0xf3,0x9e,0x1c,0x00,0x00,0x00,0x00,0xe0,0xf3,0xf8,0x03,0x00,0x00,0x00,0x00,0x00,0xfe,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
 static unsigned char u8g_go_bits[] U8G_PROGMEM = {
/*– 文字: 分贝值为 –*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0xc0,0x07,0x00,0x02,0x80,0x01,0x60,0x00,0x00,0x04,0x30,0x1f,0x00,0x01,0x60,0x00,0xe0,0x04,0xf0,0x07,0xb0,0x01,0x30,0x3c,0xa0,0x05,0x18,0x01,0xc0,0x1f,0xf8,0x30,0xb0,0x0d,0x08,0x0f,0xf8,0x30,0xdc,0x00,0xb0,0x0c,0x04,0x0b,0xc0,0x30,0xe0,0x03,0xb0,0x0c,0x06,0x0f,0xe0,0x10,0x30,0x02,0xb0,0x04,0x08,0x0f,0xe0,0x1b,0x00,0x02,0xb0,0x04,0x88,0x0f,0x30,0x18,0x00,0x03,0xc0,0x06,0x08,0x0f,0x18,0x0c,0x00,0x03,0xc0,0x07,0x9c,0x1f,0x8c,0x0e,0x80,0x01,0x30,0x0c,0xe8,0x3f,0x86,0x07,0xe0,0x01,0x38,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static unsigned char u8g_go1_bits[] U8G_PROGMEM = {
/*– 文字: 请安静 –*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x00,0xe0,0x00,0x00,0xe0,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x1c,0x00,0x00,0xe0,0x00,0x00,0xe0,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x1c,0xfe,0xff,0x3f,0x00,0xe0,0x01,0x00,0xe0,0x00,0xfe,0x03,0x00,0x00,0x00,0x00,0x3c,0xfe,0xff,0x3f,0x00,0xc0,0x01,0x00,0xff,0x1f,0xff,0x03,0x00,0x00,0x00,0x00,0x78,0xfe,0xff,0x3f,0xfe,0xff,0xff,0x1f,0xff,0x9f,0xff,0x03,0x00,0x00,0x00,0x00,0x70,0x00,0x1e,0x00,0xfe,0xff,0xff,0x1f,0xe0,0xc0,0xc3,0x03,0x00,0x00,0x00,0x00,0x60,0xfc,0xff,0x1f,0xfe,0xff,0xff,0x1f,0xe0,0xc0,0xc1,0x01,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x1f,0x0e,0x30,0x00,0x1c,0xfe,0xcf,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x1f,0x0e,0x38,0x00,0x1c,0xfe,0xcf,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x0e,0x38,0x00,0x1c,0xe0,0xc0,0xff,0x0f,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0x3f,0x06,0x3c,0x00,0x18,0xe0,0x80,0xff,0x0f,0x00,0x00,0x00,0x80,0x3f,0xff,0xff,0x7f,0x00,0x1c,0x00,0x00,0xff,0x3f,0x30,0x0c,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x3f,0x00,0x1e,0x00,0x00,0xff,0x3f,0x30,0x0c,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0xff,0xff,0xff,0x3f,0x00,0x00,0x38,0x1c,0x00,0x00,0x00,0x00,0x38,0xf8,0xff,0x8f,0xff,0xff,0xff,0x7f,0xfe,0xef,0xff,0x3f,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0xff,0xff,0xff,0x3f,0xfe,0xef,0xff,0x7f,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0x80,0x03,0xf0,0x00,0x0e,0x0e,0x38,0x1c,0x00,0x00,0x00,0x00,0x38,0x1c,0x00,0x0e,0xc0,0x03,0x70,0x00,0x0e,0x0c,0x30,0x0c,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0xc0,0x01,0x78,0x00,0xfe,0x0f,0x30,0x0c,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0xe0,0x03,0x38,0x00,0xfe,0x0f,0x38,0x0c,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0xe0,0x0f,0x1c,0x00,0xfe,0xcf,0xff,0x0f,0x00,0x00,0x00,0x00,0x38,0x1c,0x00,0x0e,0x00,0x7f,0x1e,0x00,0x0e,0xce,0xff,0x0f,0x00,0x00,0x00,0x00,0x38,0xfc,0xff,0x0f,0x00,0xfc,0x0f,0x00,0xfe,0x0f,0x38,0x0c,0x00,0x00,0x00,0x00,0x38,0xfd,0xff,0x0f,0x00,0xf0,0x0f,0x00,0xfe,0x0f,0x30,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0x0f,0x00,0xf0,0x1f,0x00,0xfe,0x0f,0x30,0x00,0x00,0x00,0x00,0x00,0xf8,0x1d,0x00,0x0e,0x00,0xfe,0xff,0x00,0x0e,0x0c,0x30,0x00,0x00,0x00,0x00,0x00,0xf8,0x1c,0x00,0x0e,0xc0,0x7f,0xfc,0x03,0x0e,0x0c,0x38,0x00,0x00,0x00,0x00,0x00,0x78,0x1c,0xf0,0x0f,0xfe,0x1f,0xf0,0x0f,0x0e,0x8e,0x3f,0x00,0x00,0x00,0x00,0x00,0x18,0x1c,0xf8,0x0f,0xfe,0x03,0xc0,0x0f,0xce,0x8f,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0xf0,0x07,0x7e,0x00,0x00,0x07,0xce,0x87,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
void loop() {
 if((analogRead(mic_pin)) > 200)
  {
      u8g.firstPage();
    do {
      u8g.drawXBMP( 0, 10, 128, 64, u8g_go1_bits);
      u8g.drawXBMP( 0, 0, 64, 25, u8g_go_bits);
      setFont_L;
      u8g.setPrintPos(66, 20);
      u8g.print (analogRead(mic_pin));
      }while( u8g.nextPage() );
  }
  else{
      u8g.firstPage();
    do {
      u8g.drawXBMP( 32, 0, 64, 64, u8g_go4_bits);
    }while( u8g.nextPage() );
  }
   delay(200);
  
   if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
  
}

void getSensorData(){  
    sensor_tem = TempMonitor.getTemperature();   
    delay(200);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    analogRead(mic_pin);
    delay(200);
    dtostrf(analogRead(mic_pin), 2, 1,mic_pin_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

    jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";                   
    jsonToSend+="}";  



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
//串口打印传感器数据
   Serial.print("DB:");
   Serial.print(analogRead(mic_pin));
   Serial.print("\t");
   Serial.print("Tem:");
   Serial.print(termo.getTemperature());  //串口打印获取的温度
   Serial.print("\t");
   Serial.println("------------------");
  
}