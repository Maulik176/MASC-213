#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <Stepper.h> 
#include <RF24.h>
#include <Servo.h> // include servo library to use its 
#define Servo_PWM 3 // A descriptive name for D6 pin of Arduino to provide PWM signal
#define STEPS 200

Stepper stepper(STEPS, 7, 5); // Pin 7 connected to DIRECTION & Pin 5 connected to STEP Pin of Driver
#define motorInterfaceType 1

int StepperRefVal = 180;
int Step_Y2 = 0;

Servo ServoY1;// Define an instance of of Servo with the name of "MG995_Servo"
Servo ServoX1;
Servo ServoY2_C;
Servo ServoY2_AC;
Servo Servo_Flex;
Servo ServoEJ;
char transmit[13];
String data="";
String xs;
String ys;
double text[6];

double x,y;

RF24 radio(9,10); // CE, CSN
const byte address[6] = "12345";
void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  //Wire.onReceive(receiveEvent); // register event
//  Wire.onRequest(requestEvent);
  Serial.begin(9600);           // start serial for output
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setChannel(10);
  radio.setPALevel(RF24_PA_MIN);
  
  ServoY1.attach(6);
  ServoX1.attach(3);
  ServoY2_AC.attach(13);
  ServoY2_C.attach(12);
  Servo_Flex.attach(2);
  ServoEJ.attach(4);
  stepper.setSpeed(1000);
}

void loop()
{
  radio.startListening();
  
  if ( radio.available() ) {
         radio.read( &text, sizeof(text) );
  }
  for(int i=0; i<6; i++)
  {
    Serial.print(text[i]);
    Serial.print(", ");
  }
  Serial.println();
  //double Y1 = map(text[1],4,363,180,0);
  //double Y1 = abs(180 - text[1]);
  if(text[1]>=0 && text[1]<=170){
    double Y1 = abs(180 - text[1]);
    ServoY1.write(Y1);
    delay(100);
  }
  if(text[0]>=0 && text[0]<=180){
//    ServoX1.write(text[0]);
    ServoX1.write(abs(180-text[0]));
    delay(100);
    //    Servo_Flex.write(text[0]);
  }


  if(text[4]>=0 && text[4]<=180){
    ServoY2_C.write(abs(180 - text[4]));
    ServoY2_AC.write(text[4]);
    delay(100);
    ServoEJ.write(abs(180 - text[4]));
    delay(100);
  }
  if(text[5]!=6942.00){
    if(text[5]>=11){
      Serial.println();
      Serial.println("Flex Do");
      Servo_Flex.write(10);
      delay(100);
    }else if(text[5]<11){
//      Serial.println();
//      Serial.println("Flex Don't do anything");
      Servo_Flex.write(120); 
      delay(100);
    }
//    double flex_val = map(text[5],0,30,0,180);
//    double flex_val;
//    flex_val = text[5]*(1.25);
//    Serial.println();
//    Serial.println(flex_val);
//    Servo_Flex.write(text[5]*(1.25));
//    Servo_Flex.write(text[0]);
  }

  
//int StepperRefVal = 180;
//int Step_Y2 = 0;
  Step_Y2 = map(text[3],180,363,0,500);
  if(text[3]>180 && text[3]<275){
    stepper.step(100);
    delay(50);
  }else if(text[3]>275 && text[3]<363){
    stepper.step(-100);
    delay(100);
  }
  delay(100);
}

//void requestEvent() {
//  Wire.write(transmit); // respond with message of 6 bytes
//  // as expected by master
//}
