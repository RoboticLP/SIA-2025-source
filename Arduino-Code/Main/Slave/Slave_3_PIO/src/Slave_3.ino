#include <Wire.h>
#include <Arduino.h>
#define Spule_1 6
#define Spule_2 7
#define Spule_3 8
#define interrupt_1 2
#define interrupt_2 3

void setup() {  
  Wire.begin(3);                // join i2c bus with address #8

  Wire.onRequest(requestEvent); // register event
}

void loop() {
  //Bumper 1
  
}

// function that executes whenever data is requested by master

// this function is registered as an event, see setup()

void requestEvent() {

  Wire.write("mac upload"); // respond with message of 6 bytes
  // as expected by master
}