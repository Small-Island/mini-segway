
#include <Wire.h>
//#define SDA        (6u)
//#define SCL        (7u)

int slaveID = 0x23;


/* tinyUSB ****************************************/
// #include "Adafruit_TinyUSB.h"

// // HID report descriptor using TinyUSB's template
// // Generic In Out with 64 bytes report (max)
// uint8_t const desc_hid_report[] =
// {
//   TUD_HID_REPORT_DESC_GENERIC_INOUT(4) // communication 4 bytes
// };

// Adafruit_USBD_HID usb_hid;
/***************************************************/

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
int myVelocity = 200, nowVelocity = 0, lV = 0, llV = 0, aV = 0, aVM2 = 0;//degree per second
int setCurrentVelocity = 0;

int16_t targVelM1 = 0, targVelM2 = 0;
int buf[50];

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

/* tinyUSB ********************************************************************************************************/
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
// uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
// {
//   // not used in this example
//   return 0;
// }

// // Invoked when received SET_REPORT control request or
// // received data on OUT endpoint ( Report ID = 0, Type = 0 )
// void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
// {
//   targVelM1 = (int16_t)((buffer[0] << 8) | buffer[1]);
//   targVelM2 = (int16_t)((buffer[2] << 8) | buffer[3]);
  
//   Serial.print("bufsize: ");
//   Serial.println(bufsize);
//   for (int i = 0; i < bufsize; i++) {
//     Serial.print(buffer[i]); Serial.print(" ");
//   }
//   Serial.println();
  
//   uint8_t value_for_send_to_nx[64];// = {0, 0, 0, 0};
//   value_for_send_to_nx[0] = (uint8_t)((aV >> 8) & 0xff);
//   value_for_send_to_nx[1] = (uint8_t)(aV  & 0xff);
//   value_for_send_to_nx[2] = (uint8_t)((aVM2 >> 8) & 0xff);
//   value_for_send_to_nx[3] = (uint8_t)(aVM2 & 0xff);
//   usb_hid.sendReport(0, value_for_send_to_nx, 4);
// }
/*********************************************************************************************************************/

void setup() {
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
  
  /* tinyUSB *********************************************************/
  // usb_hid.enableOutEndpoint(true);
  // usb_hid.setPollInterval(2);
  // usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  // //usb_hid.setReportCallback(get_report_callback, set_report_callback);
  // usb_hid.setReportCallback(0, set_report_callback);
  // //usb_hid.setStringDescriptor("TinyUSB HID Generic");
  // usb_hid.begin();
  
  // Serial.begin(115200);
  // // wait until device mounted
  // while( !USBDevice.mounted() ) delay(1);

  // Serial.println("Adafruit TinyUSB HID Generic In Out example");

  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();

  lastPosition = nowPosition = getHall_Position();
  nextPos = nowPosition;

  // Wire.onRequest(requestEvent); // data request to send from Master to slave
  // Wire.onReceive(receiveEvent); // data slave recieved from Master
}

int U_out [6] = {1, 1, 0, 0, 0, 0};
int SUout [6] = {1, 1, 0, 1, 1, 0};
int V_out [6] = {0, 0, 1, 1, 0, 0};
int SVout [6] = {1, 0, 1, 1, 0, 1};
int W_out [6] = {0, 0, 0, 0, 1, 1};
int SWout [6] = {0, 1, 1, 0, 1, 1};
int duty = 100;//pmw comment 0-1023

void loop() {
  ////////////////////////////////////
  //communication with PC with Serial
  uint8_t idx = 0;
  while (Serial.available()) {
    uint8_t data = Serial.read();
    Serial.write(data);
    switch (idx) {
      case 0:
        targVelM1 = (int16_t)(data << 8);
        break;
      case 1:
        targVelM1 = targVelM1 | (int16_t)(data);
        break;
      case 2:
        targVelM2 = (int16_t)(data << 8);
        break;
      case 3:
        targVelM2 = targVelM1 | (int16_t)(data);
        break;
    }
    idx++;
  }
  // Serial.println(', ');
  ////////////////////////////////////
  //communication with slaveID
  Wire.beginTransmission(slaveID); //スレーブが存在するか確認(モータ２）
  byte error = Wire.endTransmission(); 
  if( error == 0){
    uint16_t len = 2;
    byte setTgSlv[] = {0, 0};
    setTgSlv[0] = ((int)(targVelM2) >> 8) & 0xFF;
    setTgSlv[1] = ((int)(targVelM2)) & 0xFF;
    Wire.beginTransmission(slaveID); //スタート・コンディションの発行
    for (int i = 0; i < 2; i++){
      Wire.write(setTgSlv[i]);
    }
    Wire.endTransmission(); //ストップ・コンディションの発行
    Wire.requestFrom(slaveID, len); //受信開始（スタート（ストップ）・コンディションの発行）
    uint16_t slvbuf[2];
    for( uint16_t i=0; i < len; i++ ){
      // uint8_t slvData= Wire.read(); //len分だけデータをリードする
      slvbuf[i] = Wire.read();
    }
    aVM2 = (int16_t)((slvbuf[0] << 8) | slvbuf[1]);
  }
  //////////////

  //motor control
  nowTime = millis();
  
  int nowPosition = getHall_Position();
  if (nowPosition == -1){//hall sensor error
    return;
    // Serial.println("Hall sensor error");
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
    myVelocity = targVelM1;
    nowVelocity = (int)((realPosition - lastRealPos) / 0.1);//degree per second
    aV = (int)(1.0 * (lV + llV + nowVelocity) / 3.0);
    lV = nowVelocity;
    llV = lV;
    // Serial.print(realPosition); Serial.print(", "); Serial.println(aV);

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



  
  
