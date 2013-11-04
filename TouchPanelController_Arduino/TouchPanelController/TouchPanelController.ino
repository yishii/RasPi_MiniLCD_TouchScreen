#include <Wire.h>


// control touch panel
// Author : Yasuhiro ISHII
//
// TODO : 自分の環境だけの実測メモ。あとでキャリブレーションコマンド追加する
// x : 64 - 828
// y : 116 - 750

#define PIN_X1  A0
#define PIN_X2  A1
#define PIN_Y1  A2
#define PIN_Y2  A3
#define PIN_LED 13

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define I2C_SLAVE_ADDR  0x20

int touch_x;
int touch_y;
boolean touch_invoked;
boolean touch_tapped;

void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);
  
  Serial.begin(115200);
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(i2cOnRequestHandler);
}

void i2cOnRequestHandler()
{
  unsigned char dataToSend[6];

  //Serial.println("ReqHnd");
  
  if(touch_invoked){
    dataToSend[0] = 0x01;
    dataToSend[1] = touch_x & 0xff;
    dataToSend[2] = (touch_x >> 8) & 0xff;
    dataToSend[3] = touch_y & 0xff;
    dataToSend[4] = (touch_y >> 8) & 0xff;
    dataToSend[5] = touch_tapped == true ? 0x01 : 0x00;
    touch_tapped = false;
  } else {
    memset(dataToSend,0,sizeof(dataToSend));
  }
  
  Wire.write(dataToSend,5);
}

unsigned long tapped_time;

void loop() {
  // put your main code here, to run repeatedly: 

  //char str[128];
  int x;
  int y;

  y = capturePosition(0);
  x = capturePosition(1);

  if(isInRange(x,y) == true){
    // TODO : 暫定処理。後ほどキャリブレーションモードを追加する
    touch_y = calcNormalization(y - 116,750 - 116,SCREEN_HEIGHT);
    touch_x = calcNormalization(x -  64,828 -  64,SCREEN_WIDTH);

    //sprintf(str,"(x,y) = (%4d,%4d)",touch_x,touch_y);
    //Serial.println(str);

    touch_invoked = true;
    //digitalWrite(PIN_LED,HIGH);
    
    if(tapped_time == 0){
      tapped_time = millis();
    }
    
  } else {
    touch_invoked = false;
    //digitalWrite(PIN_LED,LOW);
    if(millis() - tapped_time < 100){
      toggleLed();
      touch_tapped = true;      
    }
    tapped_time = 0;
  }
    
}

boolean isInRange(int x,int y)
{
  boolean ret = false;

  if((x > 64) && (x < 828) && (y > 116) && (y < 750)){
    ret = true;
  }

  return ret;
}

int calcNormalization(int raw_val,int raw_width,int width)
{
  unsigned long p;
  unsigned long result;

  p = (unsigned long)raw_val * 100 / raw_width;
  result = (width * p) / 100;

  return((int)result);
}

int capturePosition(int mode)
{
  int ret;

  if(mode == 0){
    pinMode(PIN_Y1,INPUT);
    pinMode(PIN_Y2,INPUT);
    pinMode(PIN_X1,OUTPUT);
    pinMode(PIN_X2,OUTPUT);
    digitalWrite(PIN_X1,LOW);
    digitalWrite(PIN_X2,HIGH);
    delay(10);
    ret = analogRead(PIN_Y1);
  } 
  else {
    pinMode(PIN_X1,INPUT);
    pinMode(PIN_X2,INPUT);
    pinMode(PIN_Y1,OUTPUT);
    pinMode(PIN_Y2,OUTPUT);
    digitalWrite(PIN_Y1,LOW);
    digitalWrite(PIN_Y2,HIGH);
    delay(10);
    ret = analogRead(PIN_X1);
  }

  return(ret);
}

void toggleLed(void)
{
  static boolean flag;
  
  if(flag == false){
    flag = true;
  } else {
    flag = false;
  }
  
  digitalWrite(PIN_LED,flag == true ? HIGH : LOW);
}

