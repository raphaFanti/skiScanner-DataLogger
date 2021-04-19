
/*
Oberalp - R&I Lab
Logging ski scanner's data

By: Raphael Fanti e Isabella Soraruf
Date: March 5th, 2021

Setup:
 - Ski scanner shield on Adalogger
 - rotary encoder on port R1 (closest to led)
 
*/

// libraries
#include <SPI.h> //lib for serial peripheral interface
#include <SD.h> //lib for sd card
//#include <PS2Mouse.h>


// mouse on port L2
// const int mouseData = A5;
// const int mouseClock = A4;
//PS2Mouse mouse(mouseData, mouseClock);

// linear encoder on port L1
const int lin1APin = 12;
const int lin1BPin = 13;
volatile long lin1Pulses = 0; // counter of pulses on lin1
const float lin1PulsesPerCm = 249.4; //acquired manually on 10 cm on sensor
float lin1Pos = 0; // (initial) position of lin1 (initial distance between arms cylinders surfaces)
const float offset = 5.72 ; // distance between pins is offset for linear position

// linear encoder on port L2
const int lin2APin = A4;
const int lin2BPin = A5;
volatile long lin2Pulses = 0; // counter of pulses on lin1
const float lin2PulsesPerCm = 249.4; //acquired manually on 10 cm on sensor
float lin2Pos = 0; // (initial) position of lin2 (initial distance between arms cylinders surfaces)

// rotary encoder on port R1
const int rot1APin = 6;
const int rot1BPin = 9;
volatile long rot1Pulses = 0; // counter of pulses on rot1
const int rot1PulsesPerRev = 500;
float rot1Pos = 0; // angle on rot1 (deg)
float dpc = 48.97; // degrees per centimeter (rot to lin conversion) BIG WHEEL

// rotary encoder on port R2
const int rot2APin = 11;
const int rot2BPin = 10;
volatile long rot2Pulses = 0; // counter of pulses on rot1
const int rot2PulsesPerRev = 500;
float rot2Pos = 0; // angle on rot1 (deg)
float dpc2 = 53.80; // degrees per centimeter (rot to lin conversion) SMALL WHEEL

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
  }
  
  // pin mode declaration
  pinMode(rot1APin, INPUT_PULLUP);
  pinMode(rot1BPin, INPUT_PULLUP);
  pinMode(lin1APin, INPUT_PULLUP);
  pinMode(lin1BPin, INPUT_PULLUP);

  pinMode(rot2APin, INPUT_PULLUP);
  pinMode(rot2BPin, INPUT_PULLUP);
  pinMode(lin2APin, INPUT_PULLUP);
  pinMode(lin2BPin, INPUT_PULLUP);
  
  //pinMode(builtinLedPin, OUTPUT); //this can't be done because pin 13 is also used as input in lin1BPin
  pinMode(externalLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // interruption declarations
  attachInterrupt(digitalPinToInterrupt(buttonPin), recordingChange, FALLING); // falling because of pullup
  attachInterrupt(digitalPinToInterrupt(rot1APin), rot1Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(lin1APin), lin1Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(rot2APin), rot2Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(lin2APin), lin2Pulse, FALLING);


  // mouse setup
  //mouse.begin();
  // SD initialisation
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed"); // sd card not found
    while (1);
  } else {
    Serial.println("Card initialised");
  }
  // delay(1000);
  
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
  
  String header = "ID,timestamps,L1,W1,L2,W2,"; // file header
  
  if (dataFile){ //if file is available write information   
    dataFile.println(header); //writing header in datafile
    dataFile.close();
    Serial.println(header); // print also sensor data to Serial
  } else {
    Serial.println("error creating datafile"); // error message if file not open
  } 
  
  delay(200);
}

void loop() {

  // update encoders position
  rot1Pos = updateRot(rot1Pulses, rot1PulsesPerRev);
  lin1Pos = updateLin(lin1Pulses, lin1PulsesPerCm);
  rot2Pos = updateRot(rot2Pulses, rot2PulsesPerRev);
  lin2Pos = updateLin(lin2Pulses, lin2PulsesPerCm);

  // get mouse position
  uint8_t stat;
  int x,y;
  //mouse.getPosition(stat,x,y);
  
  // detect recording button push actioned
  if(recording != oldRecording){

    // understands if at beginning or end of recording
    if (recording == HIGH){ // beginning

      // open file 
      dataFile = SD.open(filename,FILE_WRITE); //open dataFile 
      Serial.println("Starting_recording,");
      Serial.println(filename);
      id = 1;
      
      // reset incremental encoders
      lin1Pulses = 0;
      lin1Pos = 0;
      lin2Pulses = 0;
      lin2Pos = 0;
      rot1Pulses = 0; 
      rot1Pos = 0;
      rot2Pulses = 0; 
      rot2Pos = 0;
      
      // recording light on, standby off
      digitalWrite(externalLedPin, HIGH);
      //digitalWrite(builtinLedPin, LOW); //this can't be done because pin 13 is also used as input in lin1BPin
      
    } else{ // end

      // close file (TBD)
      dataFile.close();
      Serial.println("Ending_recording,");
      Serial.println(filename);

      // recording light off, standby on
      digitalWrite(externalLedPin, LOW);
      //digitalWrite(builtinLedPin, HIGH); //this can't be done because pin 13 is also used as input in lin1BPin
      
    }

    // update old value for flow control
    oldRecording = recording;
    
  }
  
  if (printSerial == true){
    
      Serial.println("Rotary pos1: " + String(rot1Pulses) + "  " + String(rot1Pos));
      Serial.println("Linear pos1: " + String(lin1Pulses) + "  " + String(lin1Pos));
      Serial.println("Length1: " + String(rot1Pos/dpc));
      Serial.println("Width1: " + String(offset + lin1Pos));
      
      Serial.println("Rotary pos2: " + String(rot2Pulses) + "  " + String(rot2Pos));
      Serial.println("Linear pos2: " + String(lin2Pulses) + "  " + String(lin2Pos));
      Serial.println("Length2: " + String(rot2Pos/dpc2));
      Serial.println("Width2: " + String(offset + lin2Pos));
      
  }

  if (recording == HIGH){

    unsigned long timestamp = millis();
    String dataoutput = String(rot1Pos/dpc) + ", " + String(offset + lin1Pos) + ", " + String(rot2Pos/dpc2) + ", " + String(offset + lin2Pos) + ",";
    String dataoutput_id = "ID," + String(id) + ", " + String(timestamp) + ", " + String(rot1Pos/dpc) + ", " + String(offset + lin1Pos) + ", " + String(rot2Pos/dpc2) + ", " + String(offset + lin2Pos) + ",";
   
    Serial.println(dataoutput_id); //Da cancellare
    id++; //Da cancellare
    
    if (dataoutput.equals(dataoutput_old) != true){
      dataFile.println(dataoutput_id); //print sensor data to datafile
      Serial.println(dataoutput_id); // print also sensor data to serial

      id++;
      dataoutput_old = dataoutput;
    }
 
  }

  delay(loopDelay);

}

void lin1Pulse(){
  if(digitalRead(lin1BPin) == HIGH) lin1Pulses += 1;
  else lin1Pulses -= 1;
}

void rot1Pulse(){
  if(digitalRead(rot1BPin) == HIGH) rot1Pulses += 1;
  else rot1Pulses -= 1;
}

void lin2Pulse(){
  if(digitalRead(lin2BPin) == HIGH) lin2Pulses += 1;
  else lin2Pulses -= 1;
}

void rot2Pulse(){
  if(digitalRead(rot2BPin) == HIGH) rot2Pulses += 1;
  else rot2Pulses -= 1;
}

float updateRot(long pulses, float ppr){
  return pulses * 360.0 / ppr;
}

float updateLin(long pulses, float ppcm){
  return pulses / ppcm;
}

void recordingChange() {
  const int now = millis();
  if (now - lastButtonPress >= debounce) {
    recording = !recording;
    lastButtonPress = now;
  }
}
