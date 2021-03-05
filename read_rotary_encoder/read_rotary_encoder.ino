/*
Oberalp - R&I Lab
Logging ski scanner's data

By: Raphael Fanti
Date: MArch 5th, 2021

Setup:
 - Ski scanner shield on Adalogger
 - rotary encoder on port rot1 (closest to led)
 
*/

// include libraries needed for Adalogger
#include <SPI.h> //lib for serial peripheral interface
#include <SD.h> //lib for sd card


// rotary encoder on port rot1
const int rot1APin = 6;
const int rot1BPin = 9;
long rot1Pulses = 0; // counter of pulses on rot1
const int rot1PulsesPerRev = 500;
long rot1Pos = 0; // angle on rot1 (deg)


// leds and button pin declaration
const int builtinLedPin = 13; // red led onboard (used as standby indicator)
const int externalLedPin = 20; // green led (used as recording ON indicator)
const int buttonPin = 5;

// flags
volatile byte recording = LOW;
bool oldRecording = false;

// runtime choices
const bool printSerial = true;

//chipset declaration for Adalogger / sd card
const int chipSelect = 4;

// debouncing
const int debounce = 200;
unsigned long lastButtonPress = millis();


void setup() {

  // start serial comm
  if (printSerial == true){
    Serial.begin(9600); // start serial
  }
  
  // pin mode declaration
  pinMode(rot1APin, INPUT_PULLUP);
  pinMode(rot1BPin, INPUT_PULLUP);
  
  pinMode(builtinLedPin, OUTPUT);
  pinMode(externalLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // interruption declarations
  attachInterrupt(digitalPinToInterrupt(buttonPin), recordingChange, FALLING); // falling because of pullup
  attachInterrupt(digitalPinToInterrupt(rot1APin), rot1Pulse, FALLING);

}

void loop() {

  // update encoders position
  rot1Pos = updateRot(rot1Pulses, rot1PulsesPerRev);
  
  // detect recording button push actioned
  if(recording != oldRecording){

    // understands if at beginning or end of recording
    if (recording == HIGH){ // beginning

      // create file (TBD)
      
      // recording light on, standby off
      digitalWrite(externalLedPin, HIGH);
      digitalWrite(builtinLedPin, LOW);
      
    } else{ // end

      // close file (TBD)

      // recording light off, standby on
      digitalWrite(externalLedPin, LOW);
      digitalWrite(builtinLedPin, HIGH);
      
    }

    oldRecording = recording;
    
  }

  
  

  if (printSerial == true){
    Serial.println(rot1Pos);
  }

  delay(200);

}

void rot1Pulse(){
  if(digitalRead(rot1BPin) == HIGH) rot1Pulses += 1;
  else rot1Pulses -= 1;
}

long updateRot(long pulses, float ppr){
  return pulses * 360.0 / ppr;
}

void recordingChange() {
  const int now = millis();
  if (now - lastButtonPress >= debounce) {
    recording = !recording;
    lastButtonPress = now;
  }
}
