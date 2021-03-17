
/*
Oberalp - R&I Lab
Logging ski scanner's data

By: Raphael Fanti e Isabella Soraruf
Date: MArch 5th, 2021

Setup:
 - Ski scanner shield on Adalogger
 - rotary encoder on port R1 (closest to led)
 
*/

// libraries
#include <SPI.h> //lib for serial peripheral interface
#include <SD.h> //lib for sd card
//#include <PS2Mouse.h>


// mouse on port L2
const int mouseData = A5;
const int mouseClock = A4;
//PS2Mouse mouse(mouseData, mouseClock);

// linear encoder on port L1
const int lin1APin = 12;
const int lin1BPin = 13;
volatile long lin1Pulses = 0; // counter of pulses on lin1
const float lin1PulsesPerCm = 66.6666;
long lin1Pos = 0; // (initial) position of lin1 (initial distance between arms cylinders surfaces)

// rotary encoder on port R1
const int rot1APin = 6;
const int rot1BPin = 9;
volatile long rot1Pulses = 0; // counter of pulses on rot1
const int rot1PulsesPerRev = 500;
long rot1Pos = 0; // angle on rot1 (deg)

// leds and button
const int builtinLedPin = 13; // red led onboard (used as standby indicator)
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
const int debounce = 500; // ms
unsigned long lastButtonPress = millis();

unsigned long milliseconds;
long id = 1;
char filename[11];
File dataFile;

void setup() {

  // start serial comm
  if (printSerial == true){
    Serial.begin(9600); // start serial
  }
  delay(3000); // waiting for serial setting
  
  // SD initialisation
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed"); // sd card not found
    while (1);
  } else {
    Serial.println("Card initialised");
  }
  delay(1000);
  
  // pin mode declaration
  pinMode(rot1APin, INPUT_PULLUP);
  pinMode(rot1BPin, INPUT_PULLUP);
  pinMode(lin1APin, INPUT_PULLUP);
  pinMode(lin1BPin, INPUT_PULLUP);
  
  pinMode(builtinLedPin, OUTPUT);
  pinMode(externalLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // interruption declarations
  attachInterrupt(digitalPinToInterrupt(buttonPin), recordingChange, FALLING); // falling because of pullup
  attachInterrupt(digitalPinToInterrupt(rot1APin), rot1Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(lin1APin), lin1Pulse, CHANGE);

  // mouse setup
  //mouse.begin();
  
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
  
  String header = "ID, Rotary_en_1, Linear_en_1"; // file header
  
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
      Serial.println("Starting recording to ");
      Serial.println(filename);
      id = 1;
      
      // recording light on, standby off
      digitalWrite(externalLedPin, HIGH);
      digitalWrite(builtinLedPin, LOW);
      
    } else{ // end

      // close file (TBD)
      dataFile.close();
      Serial.println("Ending recording to ");
      Serial.println(filename);

      // recording light off, standby on
      digitalWrite(externalLedPin, LOW);
      digitalWrite(builtinLedPin, HIGH);
      
    }

    // update old value for flow control
    oldRecording = recording;
    
  }
  
  if (printSerial == true){
    Serial.print("R1 position:");
    Serial.println(rot1Pos);
    Serial.print("Linear pulses:");
    Serial.println(lin1Pulses);
  }

  delay(loopDelay);

  if (recording == HIGH){
    
    String dataoutput = String(id) + ", " + String(rot1Pos) + ", " + String(lin1Pulses) + ",";
        
    dataFile.println(dataoutput); //print sensor data to datafile
    
    Serial.println(dataoutput); // print also sensor data to serial

    id++;
    
//    Serial.println("Rotary pos: " + rot1Pos);
//    Serial.println("Linear pos: " + lin1Pulses);
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
