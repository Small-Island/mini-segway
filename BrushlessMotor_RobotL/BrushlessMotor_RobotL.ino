
/*
// RGB LED Seed XIAO RP2040 --------------------------------------------------------
#include <Adafruit_NeoPixel.h>
int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//--------------------------------------------------------
*/
//hall sensor pins
const uint HU = D8;
const uint HV = D3;
const uint HW = D2;

//motor pins
int OUT_U = D10;
int OUT_V = D9;
int OUT_W = D1;
//High impedance of motor pins
int SD_U = D7;
int SD_V = D6;
int SD_W = D0;

int numPole = 15;
int nowPosition = 0, lastPosition = 0, nextPos = 0;;
long realPosition = 0, lastRealPos = 0, numCircle = 0;
long nowTime = 0, lastTime = 0, lastTimeVelo = 0;
int myVelocity = -180, nowVelocity = 0;//degree per second
int myAcceleraion = 90; // degree per second per second
int setCurrentVelocity = 0;

void setup() {
  /*
  // put your setup code here, to run once:
  // RGB LED Seed XIAO RP2040 ------
  pixels.begin();
  pinMode(Power, OUTPUT);
  digitalWrite(Power, HIGH);
  //--------------------------------
  */
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
/*
  for (int i = 0; i < 3; i++) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    delay(500);
    pixels.clear();
    pixels.show();
    delay(500);
  }
*/
  pinMode(OUT_U, OUTPUT);
  pinMode(OUT_V, OUTPUT);
  pinMode(OUT_W, OUTPUT);
  pinMode(SD_U, OUTPUT);
  pinMode(SD_V, OUTPUT);
  pinMode(SD_W, OUTPUT);

  pinMode(HU, INPUT_PULLUP);
  pinMode(HV, INPUT_PULLUP);
  pinMode(HW, INPUT_PULLUP);

  
  digitalWrite(SD_U, HIGH);
  digitalWrite(SD_V, HIGH);
  digitalWrite(SD_W, HIGH);

  lastPosition = nowPosition = getHall_Position();
  nextPos = nowPosition;
}

int U_out [6] = {1, 1, 0, 0, 0, 0};
int SUout [6] = {1, 1, 0, 1, 1, 0};
int V_out [6] = {0, 0, 1, 1, 0, 0};
int SVout [6] = {1, 0, 1, 1, 0, 1};
int W_out [6] = {0, 0, 0, 0, 1, 1};
int SWout [6] = {0, 1, 1, 0, 1, 1};
int duty = 100;
int sinAmp = 0;

void loop() {
  nowTime = millis();
  
  int nowPosition = getHall_Position();
  if (lastPosition > nowPosition + 3)
    numCircle += 1;
  else if(lastPosition < nowPosition - 3)
    numCircle -= 1;
  lastPosition = nowPosition;
  
  
  
  realPosition = (long)((numCircle * 360.0 + nowPosition * 60.0) / numPole);
  if(nowTime -  lastTimeVelo >= 100){//every 100ms
    nowVelocity = (int)((realPosition - lastRealPos) / 0.1);//degree per second
    //Serial.print(realPosition); Serial.print(", "); Serial.println(nowVelocity);
    lastRealPos = realPosition;
    lastTimeVelo = nowTime;
  }

  int dir = 0;
  if (setCurrentVelocity != 0){
    dir = (int)(setCurrentVelocity/abs(setCurrentVelocity));
  }
  setCurrentVelocity = myVelocity;
  int timeForEver60Degree = (int)(1000.0 * 60.0 / abs(setCurrentVelocity) / numPole);//to millisecond
  if(nowTime - lastTime >= timeForEver60Degree){
    nextPos = (nextPos + dir + 6) % 6;
    //Serial.print(nowPosition); Serial.print(", "); Serial.println(nextPos);
    analogWrite(OUT_U, 100 * U_out[nextPos]);
    digitalWrite(SD_U, SUout[nextPos]);
    analogWrite(OUT_V, 100 * V_out[nextPos]);
    digitalWrite(SD_V, SVout[nextPos]);
    analogWrite(OUT_W, 100 * W_out[nextPos]);
    digitalWrite(SD_W, SWout[nextPos]);
    lastTime = nowTime;
  }
}

int getHall_Position(){
  int valHU = digitalRead(HU);
  int valHV = digitalRead(HV);
  int valHW = digitalRead(HW);
  //Serial.print(valHU); Serial.print(", "); Serial.print(valHV);Serial.print(", "); Serial.println(valHW);
  if(!valHU && !valHV && valHW)//001
    return 0;//0degree
  else if(valHU && !valHV && valHW)//101
    return 1;//60degree
  else if (valHU && !valHV && !valHW)//100
    return 2;//120degree
  else if (valHU && valHV && !valHW)//110
    return 3;//180degree
  else if (!valHU && valHV && !valHW)//010
    return 4;//240 degree
  else if(!valHU && valHV && valHW)//011
    return 5; //300 degree
  else
    return -1; //no
}

  
  
