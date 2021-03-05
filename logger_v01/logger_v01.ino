// include libraries needed for Adalogger
#include <SPI.h> //lib for serial peripheral interface
#include <SD.h> //lib for sd card

// leds and button pin declaration
const int builtinLedPin = 13; // red led onboard (used as standby indicator)
const int externalLedPin = 20; // green led (used as recording ON indicator)
const int buttonPin = 5;

// flags
volatile byte recording = LOW;
bool oldRecording = false;

// runtime choices
const bool printSerial = true;
const bool writeSD = false;

//chipset declaration for Adalogger / sd card
const int chipSelect = 4;


void setup() {

  // start serial comm
  Serial.begin(9600); // start serial
  
  // pin mode declaration
  pinMode(builtinLedPin, OUTPUT);
  pinMode(externalLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // interruption declarations
  attachInterrupt(digitalPinToInterrupt(buttonPin), recordingChange, FALLING); // falling because of pullup

  // 

}

void loop() {
  
  // detect recording button push actioned
  if(recording != oldRecording){

    // debounce

    // understands if at beginning or end of recording
    if(recording == HIGH){ // beginning
      
      // recording light on, standby off
      digitalWrite(externalLedPin, HIGH);
      digitalWrite(builtinLedPin, LOW);
      
    } else{ // end


      // recording light off, standby on
      digitalWrite(externalLedPin, LOW);
      digitalWrite(builtinLedPin, HIGH);
      
    }

    oldRecording = recording;
  }

  // prints serial
  Serial.println(recording);

  delay(200);

}

void recordingChange() {
  recording = !recording;
}
