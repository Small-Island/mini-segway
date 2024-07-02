
#include <Wire.h>
//#define SDA        (6u)
//#define SCL        (7u)

int slaveID = 0x23;

//hall sensor pins
const uint HU = 8;//D8;
const uint HV = 3;//D3;
const uint HW = 2;//D2;

//motor pins
int OUT_U = 10;//D10;
int OUT_V = 9;//D9;
int OUT_W = 1;//D1;
//High impedance of motor pins
int SD_U = 7;//D7;
int SD_V = 6;//D6;
int SD_W = 0;//D0;

int numPole = 15;
int nowPosition = 0, lastPosition = 0, nextPos = 0;;
long realPosition = 0, lastRealPos = 0, numCircle = 0;
long nowTime = 0, lastTime = 0, lastTimeVelo = 0;
int myVelocity = 200, nowVelocity = 0, lV = 0, llV = 0, aV = 0;//degree per second
int setCurrentVelocity = 0;

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

int16_t targVel = 0;
int buf[50];
void receiveEvent(int numBytes) {//receive target position
  if(numBytes >= 2)
  {
    //int cmd = Wire.read();
    for(int i = 0; i < 2; i++){
      buf[i] = Wire.read();
    }
    //Serial.print(buf[0]); Serial.print(", " ); Serial.print(buf[1]); Serial.print(", " ); Serial.print(buf[2]);Serial.print(", " ); Serial.println(buf[3]);//Serial.print(", " ); Serial.println(buf[4]);
    int16_t targVel = (int16_t)((buf[1] << 8) | buf[2]);//確認必要があるｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘｘ
    //Serial.print(tg[0]); Serial.print(", " ); Serial.println(tg[1]);
  }
}

void requestEvent() { //send actual postion
  byte response[] = {0, 0};
  response[0] = ((int)(aV >> 8) & 0xFF;
  response[1] = ((int)(aV & 0xFF;
  for (int i = 0; i < 2; i++){
      Wire.write(response[i]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin(slaveID);
    
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

  Wire.onRequest(requestEvent); // data request to send from Master to slave
  Wire.onReceive(receiveEvent); // data slave recieved from Master
}

int U_out [6] = {1, 1, 0, 0, 0, 0};
int SUout [6] = {1, 1, 0, 1, 1, 0};
int V_out [6] = {0, 0, 1, 1, 0, 0};
int SVout [6] = {1, 0, 1, 1, 0, 1};
int W_out [6] = {0, 0, 0, 0, 1, 1};
int SWout [6] = {0, 1, 1, 0, 1, 1};
int duty = 100;//pmw comment 0-1023

void loop() {
  nowTime = millis();
  
  int nowPosition = getHall_Position();
  if (nowPosition == -1){//hall sensor error
    return;
    Serial.println("Hall sensor error");
  }
  else{
    if (lastPosition > nowPosition + 3)
      numCircle += 1;
    else if(lastPosition < nowPosition - 3)
      numCircle -= 1;
    lastPosition = nowPosition;
  }
  
  realPosition = (long)((numCircle * 360.0 + nowPosition * 60.0) / numPole);
  if(nowTime -  lastTimeVelo >= 100){//every 100ms
    //myVelocity = targVel;
    nowVelocity = (int)((realPosition - lastRealPos) / 0.1);//degree per second
    aV = (int)(1.0 * (lV + llV + nowVelocity) / 3.0);
    lV = nowVelocity;
    llV = lV;
    Serial.print(realPosition); Serial.print(", "); Serial.println(aV);

    if(myVelocity > 0){
      setCurrentVelocity += 40;
      if(setCurrentVelocity > myVelocity) setCurrentVelocity = myVelocity;
    }
    else{
      setCurrentVelocity -= 40;
      if(setCurrentVelocity < myVelocity) setCurrentVelocity = myVelocity;
    }
    
    if (abs(aV) < abs(myVelocity/2.0)){
      duty = 300;
    }
    else{
      duty = 150;
    }
    
    
    lastRealPos = realPosition;
    lastTimeVelo = nowTime;
  }

  int dir = 0;
  if (setCurrentVelocity != 0){
    dir = (int)(setCurrentVelocity/abs(setCurrentVelocity));
  }
  
  int timeForEver60Degree = (int)(1000.0 * 60.0 / abs(setCurrentVelocity) / numPole);//to millisecond
  if(nowTime - lastTime >= timeForEver60Degree){
    if(dir != 0){
      nextPos = (nextPos + dir + 6) % 6;
    }
    //Serial.print(nowPosition); Serial.print(", "); Serial.println(nextPos);
    pwm(OUT_U, 3000, duty * U_out[nextPos]);//analogWrite(OUT_U, duty * U_out[nextPos]);
    digitalWrite(SD_U, SUout[nextPos]);
    pwm(OUT_V, 3000, duty * V_out[nextPos]);//analogWrite(OUT_V, duty * V_out[nextPos]);
    digitalWrite(SD_V, SVout[nextPos]);
    pwm(OUT_W, 3000, duty * W_out[nextPos]);//analogWrite(OUT_W, duty * W_out[nextPos]);
    digitalWrite(SD_W, SWout[nextPos]);
    lastTime = nowTime;
  }
}



  
  
