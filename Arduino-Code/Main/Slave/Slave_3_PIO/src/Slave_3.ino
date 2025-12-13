#include <Wire.h>
#include <Arduino.h>
#define Spule_1 6
#define Spule_2 7
#define Spule_3 8
#define interrupt_1 2
#define interrupt_2 3

int numberOfBumpers = 3;
int bumper[3] = {Spule_1,Spule_2,Spule_3};

int bumperHits = 0;

String error_module3 = "";

char requestEventAnswer[50]; //text thats 50 bytes long to send the master module

void setup() {  
  Wire.begin(3);                // join i2c bus with address #8

  Wire.onRequest(requestEvent); // register event

  for(int i = 0; i < numberOfBumpers; i++){ //configure the pins that control the coils
    pinMode(bumper[i],OUTPUT);
  }
}

void loop() {
  //Bumper 1
  
}

//method to trigger a bumper, executed by the interrupt pins in  void loop
void triggerBumper(int number){
  digitalWrite(bumper[number],HIGH);
  bumperHits ++;
}

// function that executes whenever data is requested by master

// this function is registered as an event, see setup()

void requestEvent() {
  int ht1Dummy = bumperHits;
  snprintf(requestEventAnswer,sizeof(requestEventAnswer),"ht1:%d|ht2:-1|err:%s",ht1Dummy,error_module3);

  Wire.write(requestEventAnswer); // respond with message of 6 bytes

  bumperHits -= ht1Dummy;
  ht1Dummy == 0;
  // as expected by master
}