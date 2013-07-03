/*
 HC-SR04 sensor controller for quad copter.
 */
 
#include <Wire.h>  // Include I2C library

#define DEBUG 0

#define trigUpPin 10
#define echoUpPin 11
#define trigDownPin 12
#define echoDownPin 13
/*
#define accuCell1Pin A0
#define accuCell2Pin A1
#define accuCell3Pin A2

#define buzzerPin 3
#define alarmLedPin A3

#define accuTreshold 330    // Treshold off accu cells
*/
#define slaveAddress 0x42

#define numMeas 4 //Define messuring times for an avg value

// global vars
int tempDistUp[numMeas], tempDistDown[numMeas];
int index = 0;
int totalDistUp = 0;
int totalDistDown = 0;
int distanceUp = 0;
int distanceDown = 0;
byte data[4];
  

void setup() {
  // setup serial port
  Serial.begin (9600);
  
  // define pin configuration
  pinMode(trigUpPin, OUTPUT);
  pinMode(echoUpPin, INPUT);
  pinMode(trigDownPin, OUTPUT);
  pinMode(echoDownPin, INPUT);
  /*
  pinMode(buzzerPin, OUTPUT);
  pinMode(alarmLedPin, OUTPUT);
  */
  // init all temp distances to 0
  for (int c = 0; c < numMeas; c++) {
    tempDistUp[c] = 0;
    tempDistDown[c] = 0;
  }
  
  // initialize i2c as slave
  Wire.begin(slaveAddress);
   
  // define callbacks for i2c communication
  Wire.onRequest(sendData); 
}

/******************************************************/
/* Main Loop                                          */
/******************************************************/
void loop() {
  
  // calc and get up distance
  totalDistUp -= tempDistUp[index];
  tempDistUp[index] = getDistance(trigUpPin, echoUpPin);
  totalDistUp += tempDistUp[index];
  distanceUp = totalDistUp / numMeas;
  
  delay(10);
  
  // calc and get down distance
  totalDistDown -= tempDistDown[index];
  tempDistDown[index] = getDistance(trigDownPin, echoDownPin);
  totalDistDown += tempDistDown[index];
  distanceDown = totalDistDown / numMeas;
  
  index ++;
  
  if (index >= numMeas) index = 0;
  
    
  data[0] = distanceUp &0xFF;
  data[1] = (distanceUp >> 8) &0xFF;
  data[2] = distanceDown &0xFF;
  data[3] = (distanceDown >> 8) &0xFF;
  
/*  data[0] = 0x34;
  data[1] = 0x35;
  data[2] = 0x36;
  data[3] = 0x37;
*/  
  // send data
  sendDistance("Up ",distanceUp);
  sendDistance(" Down ",distanceDown);
  
  /*
  // Check accu
  int lowestCell = messAccuCells();
  if (lowestCell < accuTreshold){
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(alarmLedPin, HIGH);
    Serial.print(" Buzzer aan ");
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(alarmLedPin, LOW);
    Serial.print(" Buzzer uit ");
  }
  
  Serial.print(lowestCell);
  
  #if DEBUG
  Serial.println();
  #endif
  */
  delay(10);
}
/******************************************************/
/* Funct:   messAccuCells                              */
/*----------------------------------------------------*/
/* Input:   -                                         */
/* Output:  lowestCell                                */
/******************************************************/
/*
int messAccuCells() {
  int inputCell1, inputCell2, inputCell3;
  
  inputCell1 = map(analogRead(accuCell1Pin), 0, 1023, 0, 500);
  inputCell2 = map(analogRead(accuCell2Pin), 0, 1023, 0, 1000)-inputCell1;
  inputCell3 = map(analogRead(accuCell3Pin), 0, 1023, 0, 1500)-inputCell1-inputCell2;
  
  #if DEBUG
  Serial.print(" Cell1=");
  Serial.print(inputCell1);
  Serial.print(" Cell2=");
  Serial.print(inputCell2);
  Serial.print(" Cell3=");
  Serial.print(inputCell3);
  #endif
  
  return min(min(inputCell1, inputCell2), inputCell3); 
}
*/
/******************************************************/
/* Funct:   getDistance                               */
/*----------------------------------------------------*/
/* Input:   trigPin                                   */
/*          echoPin                                   */
/* Output:  duration                                  */
/******************************************************/
int getDistance(int trigPin, int echoPin) {
  int duration, distance;
  
  // Get distance from HC-SR04
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance += duration/58.2;
  
  return distance;
}

/******************************************************/
/* Funct:   sendDistance                              */
/*----------------------------------------------------*/
/* Input:   name                                      */
/*          distance                                  */
/* Output:  -                                         */
/******************************************************/
void sendDistance(char name[10], int distance) { 
  #if DEBUG
  Serial.print(name);
  #endif
  if (distance >= 4000 || distance <= 0){
    #if DEBUG
    Serial.println("Out of range");
    #endif
  }
  else {
    #if DEBUG
    Serial.print(distance);
    Serial.print(" cm");
    #endif
  }
}

/******************************************************/
/* Funct:   sendData                                  */
/*----------------------------------------------------*/
/* Input:   -                                         */
/* Output:  -                                         */
/******************************************************/
void sendData() {
  
  Wire.write(data,4);
  
}
