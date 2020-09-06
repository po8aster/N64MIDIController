/*
  N64 MIDI Controller v0.1
  By Po8aster
  Based on the N64Controller example from the Nintendo library by NicoHood
 
  This Test Code is Designed to:
  Send MIDI via 5 Pin on Channel 1
  Send MIDI via USB on Channel 2
  When A is pressed, send C3 in Drum Mode, send C4 in Synth Mode
  Press button on pin 10 to change mode

  To Do:
  Complete the note mapping
  Clean up comments
  Full testing for responsiveness/usability

  Required Libraries (as named in Library Manager):
  Nintendo by NicoHood
  MIDI Library by Francois Best
  MIDIUSB by Gary Grewal
*/

//Include the Nintendo Library
#include "Nintendo.h"

//Include USB and Traditional MIDI Libraries
#include "MIDIUSB.h"
#include "MIDI.h"

//This makes the traditional MIDI Work
MIDI_CREATE_DEFAULT_INSTANCE();

// Define a N64 Controller and the pin for data connection
CN64Controller N64Controller(7);

// Pin definitions
//#define pinLed LED_BUILTIN
int drumLED = 3;
int synthLED = 4;
int buttonPin = 10;

//Mode bool definitions
bool drumMode = true;
bool synthMode = false;
bool timeout = false;

//*** MIDI Functions
//Note Functions
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
//CC Function
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

//Mode Toggle Function

void toggleMode(){
  // set timeout
  timeout=true;

  //Send all notes off (CC 123) to prevent held notes
  //Traditional
  MIDI.sendControlChange(123,0,1); //CC, value, channel (1-16)
  //USB
  controlChange(1, 123, 0); //channel (0-15), CC, value
  MidiUSB.flush();
  
  //If in drum mode, switch to synth and change LEDs
  if(drumMode==true){
    drumMode = false;
    synthMode = true;
    digitalWrite(drumLED, LOW);
    digitalWrite(synthLED, HIGH);
  }
  //If in synth mode, switch to drum
  else if(synthMode==true){
    drumMode = true;
    synthMode = false;
    digitalWrite(drumLED, HIGH);
    digitalWrite(synthLED, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  
  // Set up LEDs
  //pinMode(pinLed, OUTPUT);
  pinMode(drumLED, OUTPUT);
  pinMode(synthLED, OUTPUT);

  //Set up mode button
  pinMode(buttonPin, INPUT_PULLUP);

  // Start debug serial
  // while (!Serial);

  //start traditional MIDI on Channel 1
  MIDI.begin(1);

  //Set the default LED
  digitalWrite(drumLED, HIGH);
}

void loop()
{
  // Try to read the controller data
  if (N64Controller.read())
  {
    // Print Controller information
    auto status = N64Controller.getStatus();
    auto report = N64Controller.getReport();
    print_n64_report(report, status);
  }
  else
  {
     //Add debounce if reading failed
     Serial.println(F("Error reading N64 controller."));
     delay(1500);
  }
}

void print_n64_report(N64_Report_t &n64_report, N64_Status_t &n64_status)
{
  // Print device information
  //Serial.print(F("Device: "));
  switch (n64_status.device) {
    case NINTENDO_DEVICE_N64_NONE:
      //Serial.println(F("No N64 Controller found!"));
      break;
    case NINTENDO_DEVICE_N64_WIRED:
      //Serial.println(F("Original Nintendo N64 Controller"));
      break;

    default:
      //Serial.print(F("Unknown "));
      //Serial.println(n64_status.device, HEX);
      break;
  }

//Mode Button Trigger
//read button value
  int buttonVal = digitalRead(buttonPin);
  //Serial.println(buttonVal);
  //If button pressed (0) and timeout expired, switch mode
  if(buttonVal==0 && timeout==false){
    toggleMode();
    //release timeout after short delay
    delay (500);
    timeout=false;
  }
//***Drum Mode Code
if(drumMode==true){
//A Button Trigger
  if(n64_report.a==1){
    Serial.println ("A Pressed!");
    //If button pressed send note
    //Send on USB
   noteOn(1, 48, 64);   // Channel (0-15), pitch, velo
   MidiUSB.flush();
   //Send on Traditional MIDI
   MIDI.sendNoteOn(48, 64, 1); // pitch, velo, channel (1-16)
 } else{
 //If button is not pressed, send note off
 //NoteOff for USB
  noteOff(1, 48, 64);  // Channel (0-15), pitch, velo
  MidiUSB.flush();
 //NoteOff for Traditional MIDI
  MIDI.sendNoteOff(48, 64, 1); // pitch, velo, channel (1-16)
 }

}

//***Synth Mode Code
else if (synthMode==true){
  //A Button Trigger
  if(n64_report.a==1){
    Serial.println ("A Pressed!");
    //If button pressed send note
    //Send on USB
   noteOn(1, 60, 64);   // Channel (0-15), pitch, velo
   MidiUSB.flush();
   //Send on Traditional MIDI
   MIDI.sendNoteOn(60, 64, 1); // pitch, velo, channel (1-16)
 } else{
 //If button is not pressed, send note off
 //NoteOff for USB
  noteOff(1, 60, 64);  // Channel (0-15), pitch, velo
  MidiUSB.flush();
 //NoteOff for Traditional MIDI
  MIDI.sendNoteOff(60, 64, 1); // pitch, velo, channel (1-16)
 }
 
}
 
  // Prints the raw data from the controller
  //Serial.println();
  //Serial.println(F("Printing N64 controller report:"));

  //Serial.print(F("A:\t"));
  //Serial.println(n64_report.a); 

  //Serial.print(F("B:\t"));
  //Serial.println(n64_report.b);

 // Serial.print(F("Z:\t"));
 // Serial.println(n64_report.z);

 // Serial.print(F("Start:\t"));
 // Serial.println(n64_report.start);

 // Serial.print(F("Dup:\t"));
 // Serial.println(n64_report.dup);
 // Serial.print(F("Ddown:\t"));
 // Serial.println(n64_report.ddown);
 // Serial.print(F("Dleft:\t"));
 // Serial.println(n64_report.dleft);
 // Serial.print(F("Dright:\t"));
 // Serial.println(n64_report.dright);

 // Serial.print(F("L:\t"));
 // Serial.println(n64_report.l);
 // Serial.print(F("R:\t"));
 // Serial.println(n64_report.r);

 // Serial.print(F("Cup:\t"));
 // Serial.println(n64_report.cup);
 // Serial.print(F("Cdown:\t"));
 // Serial.println(n64_report.cdown);
 // Serial.print(F("Cleft:\t"));
 // Serial.println(n64_report.cleft);
 // Serial.print(F("Cright:\t"));
 // Serial.println(n64_report.cright);

 // Serial.print(F("xAxis:\t"));
//  Serial.println(n64_report.xAxis, DEC);
//  Serial.print(F("yAxis:\t"));
//  Serial.println(n64_report.yAxis, DEC);

  //Serial.println();
}
