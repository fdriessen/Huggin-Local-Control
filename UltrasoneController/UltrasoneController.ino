/*
 HC-SR04 sensor controller for quad copter.
 */
 
#include <Wire.h>  // Include I2C library

#define trigUpPin 10
#define echoUpPin 11
#define trigDownPin 12
#define echoDownPin 13

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
  
  
  
  Serial.println();
  
  delay(10);
}

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
  Serial.print(name);
  if (distance >= 4000 || distance <= 0){
    Serial.println("Out of range");
  }
  else {
    Serial.print(distance);
    Serial.print(" cm");
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
