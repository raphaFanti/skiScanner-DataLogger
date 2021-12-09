
/*
Oberalp - R&I Lab
Logging ski scanner's data

By: Raphael Fanti e Isabella Soraruf
Date: March 5th, 2021
Last update: December 9th, 2021

Setup:
 - Ski scanner shield on Adalogger
 - rotary encoder on port R1 (closest to led)
 
*/

// libraries
#include <SPI.h> //lib for serial peripheral interface
#include <SD.h> //lib for sd card
//#include <PS2Mouse.h>

// rotary encoder on port L1 (schematics L1) rotary sensor
const int wrot1APin = A4; // 
const int wrot1BPin = A5; // 
volatile long wrot1Pulses = 0; // counter 
const int wrot1PulsesPerRev = 8192;
float wrot1Pos = 0; // (initial) position of lin1 (initial distance between arms cylinders surfaces)

const float width_offset = 5.56 ; //latest version: 58.6 // distance between pins is offset for linear position

// rotary encoder on port W2 (schematics W2) linear width sensor 2
const int wrot2APin = 6; // 12; 
const int wrot2BPin = 9; // 13;  
volatile long wrot2Pulses = 0; // counter of pulses
const int wrot2PulsesPerRev = 8192;
float wrot2Pos = 0; // (initial) position of lin1 (initial distance between arms cylinders surfaces)

// rotary encoder on port W1 (schematics W1) linear width sensor 1
const int wrot3APin = 10; //6;  
const int wrot3BPin = 11; //9;  
volatile long wrot3Pulses = 0; // counter of pulses
const int wrot3PulsesPerRev = 8192;
float wrot3Pos = 0; // (initial) position of lin1 (initial distance between arms cylinders surfaces)

float dpc = 38.20; // latest version: 38.20 degrees per centimeter // 46.84084; 47.9808; 48.97;  degrees per centimeter (rot to lin conversion) SMALL WHEEL

// leds and button
//const int builtinLedPin = 13; // red led onboard (used as standby indicator) //this can't be done because pin 13 is also used as input in lin1BPin
const int externalLedPin = 20; // green led (used as recording ON indicator)
const int buttonPin = 5;

// flags
volatile byte recording = LOW;
bool oldRecording = false;

// runtime choices
const bool printSerial = true;
const int loopDelay = 50; //ms
const int chipSelect = 4; //chipset declaration for Adalogger / sd card

// debouncing
const int debounce = 200; // ms
unsigned long lastButtonPress = millis();

unsigned long milliseconds;
long id = 1;
char filename[11];
String dataoutput_old;
File dataFile;

void setup() {

  // start serial comm
  if (printSerial == true){
    Serial.begin(9600); // start serial
    // reset incremental encoders
      wrot1Pulses = 0;
      wrot1Pos = 0;
      wrot2Pulses = 0;
      wrot2Pos = 0;
      wrot3Pulses = 0; 
      wrot3Pos = 0;
  }
  
  // pin mode declaration
  pinMode(wrot1APin, INPUT_PULLUP);
  pinMode(wrot1BPin, INPUT_PULLUP);
  
  pinMode(wrot2APin, INPUT_PULLUP);
  pinMode(wrot2BPin, INPUT_PULLUP);

  pinMode(wrot3APin, INPUT_PULLUP);
  pinMode(wrot3BPin, INPUT_PULLUP);
  
  //pinMode(builtinLedPin, OUTPUT); //this can't be done because pin 13 is also used as input in lin1BPin
  pinMode(externalLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // interruption declarations
  attachInterrupt(digitalPinToInterrupt(buttonPin), recordingChange, FALLING); // falling because of pullup
  attachInterrupt(digitalPinToInterrupt(wrot1APin), wrot1Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(wrot2APin), wrot2Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(wrot3APin), wrot3Pulse, FALLING);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed"); // sd card not found
    while (1);
  } else {
    Serial.println("Card initialised");
  }
  // delay(1000);
  delay(200);
}

void loop() {

  // update encoders position
  wrot1Pos = updateRot(wrot1Pulses, wrot1PulsesPerRev);
  wrot2Pos = updateRot(wrot2Pulses, wrot2PulsesPerRev);
  wrot3Pos = updateRot(wrot3Pulses, wrot3PulsesPerRev);
  
  // detect recording button push actioned
  if(recording != oldRecording){

    // understands if at beginning or end of recording
    if (recording == HIGH){ // beginning
      
        // Create file with file header
        // Construct filename (maxiumum digit # of filename = 12)
        
        // Setting new filename
        strcpy(filename, "prova00.TXT");
        for (uint8_t i = 0; i < 100; i++) {
          filename[5] = '0' + i/10;
          filename[6] = '0' + i%10;
          //create if does not exist, do not open existing, write, sync after write
          if (!SD.exists(filename)) {
            break;
          }
      
        }  
      
        // Writing file header
        dataFile = SD.open(filename,FILE_WRITE); //open file 
        
        String header = "ID,timestamps,L1,W2,W1,"; // file header
        
        if (dataFile){ //if file is available write information   
          dataFile.println(header); //writing header in datafile
          dataFile.close();
          Serial.println(header); // print also sensor data to Serial
        } else {
          Serial.println("error creating datafile"); // error message if file not open
        } 
  
      // open file 
      dataFile = SD.open(filename,FILE_WRITE); //open dataFile 
      Serial.println("Starting_recording,");
      Serial.println(filename);
      id = 1;
      
      // reset incremental encoders
      wrot1Pulses = 0;
      wrot1Pos = 0;
      wrot2Pulses = 0;
      wrot2Pos = 0;
      wrot3Pulses = 0; 
      wrot3Pos = 0;
      
      // recording light on
      digitalWrite(externalLedPin, HIGH);
      
    } else{ // end

      // close file (TBD)
      dataFile.close();
      Serial.println("Ending_recording,");
      Serial.println(filename);

      // recording light off
      digitalWrite(externalLedPin, LOW);
      
    }

    // update old value for flow control
    oldRecording = recording;
    
  }
  
  if (printSerial == true){
      
      Serial.println("Rotary pos1 (width1): " + String(wrot1Pulses) + "  " + String(wrot1Pos));
      Serial.println("Rotary pos2 (width2): " + String(wrot2Pulses) + "  " + String(wrot2Pos));
      Serial.println("Rotary pos3 (advancement): " + String(wrot3Pulses) + "  " + String(wrot3Pos));
  
  }

  if (recording == HIGH){

    unsigned long timestamp = millis();
    String dataoutput = String(wrot1Pulses) + ", " + String(wrot2Pulses) + ", " + String(wrot3Pulses) + "," ; //String(rot1Pos) + ", " + String(offset + lin1Pos) + ", " + String(rot2Pos) + ", " + String(offset + lin2Pos) + ",";
    String dataoutput_id = "ID," + String(id) + ", " + String(timestamp) + ", " + String(wrot1Pulses) + ", " + String(wrot2Pulses) + ", " + String(wrot3Pulses) + "," ; //+ String(offset + lin2Pos) + ","
    String dataoutput_sd = String(id) + ", " + String(timestamp) + ", " + String(wrot1Pulses) + ", " + String(wrot2Pulses) + ", " + String(wrot3Pulses) + "," ;
    
    //Serial.println(dataoutput_id); //Da cancellare
    //id++; //Da cancellare
    
    if (dataoutput.equals(dataoutput_old) != true){
      dataFile.println(dataoutput_sd); //print sensor data to datafile
      Serial.println(dataoutput_id); // print also sensor data to serial

      id++;
      dataoutput_old = dataoutput;
    }
 
  }

  delay(loopDelay);

}

void wrot1Pulse(){
  if(digitalRead(wrot1BPin) == HIGH) wrot1Pulses += 1;
  else wrot1Pulses -= 1;
}

void wrot2Pulse(){
  if(digitalRead(wrot2BPin) == HIGH) wrot2Pulses += 1;
  else wrot2Pulses -= 1;
}

void wrot3Pulse(){
  if(digitalRead(wrot3BPin) == HIGH) wrot3Pulses += 1;
  else wrot3Pulses -= 1;
}

float updateRot(long pulses, float ppr){
  return pulses * 360.0 / ppr;
}

void recordingChange() {
  const int now = millis();
  if (now - lastButtonPress >= debounce) {
    recording = !recording;
    lastButtonPress = now;
  }
}
