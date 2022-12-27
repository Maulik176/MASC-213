#include<Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7,8); //for nrf (digital pins)
const byte address[6] = "12345"; //reciever side should be same
// pins used for MPU --> A4,A5,GND,Vcc
const int MPU1 = 0x68; //specific board
//First MPU6050
int16_t AcX1, AcY1, AcZ1, Tmp1, GyX1, GyY1, GyZ1;
int minVal = 265;
int maxVal = 402;
double x;
double y;
double z;

double flex_final;

const int MPU2 = 0x69;
// Second MPU
int16_t AcX2, AcY2, AcZ2, Tmp2, GyX2, GyY2, GyZ2;
int minVal2 = 265;
int maxVal2 = 402;
double x2;
double y2;
double z2;

double ans[6]; // Array for the ARM.d

char ans_stm[13]; // String for the STM.
int idx = 0;
//char index[8][10] = {"flex1","flex2","x1","y1","z1","x2","y2","z2"};
//char index[8][10] = {"x1","y1","z1","x2","y2","z2","flex1","flex2"};
//char index[6][10] = {"x1","y1","z1","x2","y2","z2"};
char index[7][10] = {"x1","y1","z1","x2","y2","z2","flex1"}; //the order of data
int response_time = 100;

// Pins used for flex --> A0,A1, Vcc,GND.
// flex sensor variables
const int FLEX_PIN1 = A0; 
const int FLEX_PIN2 = A1; 
const float VCC = 4.98; // need to check for Nano
const float R_DIV = 47500.0; // need to varify this 

const float STRAIGHT_RESISTANCE1 = 37300.0; // resistance when straight // Unknown for our Flex sensor
const float BEND_RESISTANCE1 = 90000.0; // resistance at 90 deg // Unknown for our Flex sensor

const float STRAIGHT_RESISTANCE2 = 37300.0; // resistance when straight // Unknown for our Flex sensor
const float BEND_RESISTANCE2 = 90000.0; // resistance at 90 deg // Unknown for our Flex sensor
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Wire.beginTransmission(MPU1);
  Wire.write(0x6B);// PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU2);
  Wire.write(0x6B);// PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  delay(1000);
  pinMode(7,OUTPUT);// State --> 1 ON
                    // State --> 2 OFF
  pinMode(4,INPUT); // Switch
                    // For the state control of state --> 0 and 1
                    // 1 -> Normal ARM control LED --> ON --> RED
                    // 2 -> Controlling the Lower Base --> OFF --> RED
  // Config for NRF24L01
  radio.begin();
  radio.openWritingPipe(address);
  radio.setChannel(10);
  radio.setPALevel(RF24_PA_MIN); //pipe established
  radio.stopListening();
}
void loop() {
//  digitalRead(4) == HIGH
  if(0){ // switch that is used to convert stm state --> 1          
//    ans[5] = 6942; //dev-stm
//    ans[5] = flex_final;
    GetMpuValue1(MPU1);
    delay(10);
    GetMpuValue2(MPU2);
    delay(10);
    flex_pwm();
    if(ans!=NULL){
      radio.write(ans, sizeof(ans));
      for(int i=0;i<7;i++){
        Serial.print(index[i]);
        Serial.print(" = ");
        Serial.print(ans[i]);
        Serial.print(", ");
      }
      Serial.println();
    }
    delay(500);
    delay(500);
  }
  else{ // ARM state --> 0; //this is for the flex sensors to work when button is not pressed. 
    ans[5] = flex_final;
    GetMpuValue1(MPU1);
    delay(10);
    GetMpuValue2(MPU2);
    delay(10);
    flex_pwm();
    if(ans!=NULL){
      radio.write(ans, sizeof(ans));
      for(int i=0;i<7;i++){
        Serial.print(index[i]);
        Serial.print(" = ");
        Serial.print(ans[i]);
        Serial.print(", ");
      }
      Serial.println();
    }
    delay(500);
  }
}

void GetMpuValue1(const int MPU) {

  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false); //turning on connection, initially is true
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers

  AcX1 = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY1 = Wire.read() << 8 |  Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ1 = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  Tmp1 = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)

  int xAng = map(AcX1, minVal, maxVal, -90, 90);
  int yAng = map(AcY1, minVal, maxVal, -90, 90);
  int zAng = map(AcZ1, minVal, maxVal, -90, 90);

  GyX1 = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY1 = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ1 = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI) + 4; //offset by 4 degrees to get back to zero
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
 

  ans[0] = x;
  ans[1] = y;
  ans[2] = z;

  // Getting the data for the base ans size is 13, explanation is below

  String xdata = String(x,DEC);
  String ydata = String(y,DEC);
  xdata = xdata.substring(0,6); //removing 0's and ?'s
  ydata = ydata.substring(0,6);

  char xData[6]; 
  char yData[6];
  xdata.toCharArray(xData,6); //to chararray -- string->char because string was giving unncessary data
  ydata.toCharArray(yData,6);
  int k = 0;
  for(k=0;k<6;k++){
    ans_stm[idx] = xData[idx];  //idx is used to transfer data, problem faced here was caching (holding value)
    idx++;
  }
  ans_stm[idx] = ',';
  idx++;
  for(k=7;k<13;k++){ //365.00 and ',' so 6 indexes are taken
    ans_stm[idx] = yData[idx-7];   
    idx++;
  }
}
void GetMpuValue2(const int MPU) {

  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX2 = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY2 = Wire.read() << 8 |  Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ2 = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  Tmp2 = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)

  int xAng2 = map(AcX2, minVal2, maxVal2, -90, 90);
  int yAng2 = map(AcY2, minVal2, maxVal2, -90, 90);
  int zAng2 = map(AcZ2, minVal2, maxVal2, -90, 90);

  GyX2 = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY2 = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ2 = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  x2 = RAD_TO_DEG * (atan2(-yAng2, -zAng2) + PI) + 4; //offset by 4 degrees to get back to zero
  y2 = RAD_TO_DEG * (atan2(-xAng2, -zAng2) + PI);
  z2 = RAD_TO_DEG * (atan2(-yAng2, -xAng2) + PI);

  ans[3] = x2;
  ans[4] = y2;

}

void flex_pwm(){
  int flexADC1 = analogRead(FLEX_PIN1);
  int flexADC2 = analogRead(FLEX_PIN2);
flex_final = 0;
  delay(100);
}
