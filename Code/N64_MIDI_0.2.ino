/*
  N64 MIDI Controller v0.2
  By Po8aster
  Based on the N64Controller example from the Nintendo library by NicoHood
 
  This Test Code is Designed to:
  Send MIDI via 5 Pin on Channel 1 in Synth Mode, Channel 10 in Drum Mode
  Send MIDI via USB on Channel 2, Channel 10 in Drum Mode
  When A is pressed, send C3 in Drum Mode, send C4 in Synth Mode
  Press button on Pin 10 to change mode
  Use Joystick to control Velocity (up/down in both modes), Modulation (l/r in Drum Mode), and Pitch Bend (l/r in Synth Mode)

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
#include <pitchToNote.h>  // include pitch chart from MIDIUSB library

//This makes the traditional MIDI Work
MIDI_CREATE_DEFAULT_INSTANCE();


//*** Definitions to Update Settings/Channels/Etc
//Intended to be easily modifiable to customize the controller to your needs/preferences

// Define a N64 Controller and the pin for data connection
CN64Controller N64Controller(7); //change this number to match pin the controller data line is connected to

// Pin definitions (only change these is you built yours differently than my schematic)
int drumLED = 3; // pin for the drum mode LED
int synthLED = 4; // pin for the synth mode LED
int buttonPin = 10; // pin for the mode change button

//MIDI Channel Definitions
int drumChU = 9; //channel for USB in Drum Mode (0-15)
int drumChT = 10; //channel for Traditional in Drum Mode (1-16)
int synthChU = 1; //channel for USB in Synth Mode (0-15)
int synthChT = 1; //channel for Traditional in Synth Mode (1-16)
byte pbUSB = 0xE1; // set last digit to channel (0-15) for pitch bend messages via USB (match synthChU in most cases)ie 0xE1 for Channel 2
byte pbT = synthChT; // channel for Traditional Pitch Bend messages (defaults to same channel as notes, manually change to 1-16 if different channel desired)

//Drum Mode CC Definitions (defaults to Modulation control)
//Change these as needed to change the function of the Joystick X axis in Drum Mode
int controlVal = 1; //sets which CC message is controlled by the X axis of the joystick in Drum Mode (default 1, Modulation)

//When true the joystick will default to zero. Pushing in either direction will move the value up towards the maximum control value. When false the joystick will default to the midpoint between min and max values. Pushing the stick left will make the value go down, right will go up.
//TL;DR if your control defaults to 0, leave True. If your control defaults to 64, change to False.
bool alwaysUp = true; 

//Change these to adjust sensitivity (note that if alwaysUp is False, your control will default to the center value between these points)
// Set 1/127 for a control with a neutral value of 64 without rounding issues
int minVal = 1; //minimum value to send for selected control
int maxVal = 127; //maximum value to send for selected control

int ccChU = drumChU; //sets channel for sending the CC message on USB in Drum Mode. Defaults to Drum Mode Channel but can be manually changed to 0-15 if desired
int ccChT = drumChT; //sets channel for CC on Traditional MIDI in Drum Mode. Defaults to same channel as the Mode but can be manually changed to 1-16 if desired

//Velocity Definitions
int vel = 64; //Default Velocity at startup, will not change performance
int velFloor = 25; //Set velocity floor (giving full 0-127 range makes downward very sensitive. Reduce this value to increase sensitivity, increase to reduce sensitivity). Neutral velocity will change to midpoint between floor and 127

//Pitch Bend Definitions (set high and low limits for pitch bend, 4096/12288 1 semitone, 0/16383 is max)
int bendLow = 4096;
int bendHigh = 12288;

// *** Here Be Dragons, the below is not intended to be easily modified but is where you will go to change how the thing works instead of simple tweaks

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

//Pitch Bend Function
void midiCommand(byte cmd, byte data1, byte  data2) {
  // First parameter is the event type (top 4 bits of the command byte).
  // Second parameter is command byte combined with the channel.
  // Third parameter is the first data byte
  // Fourth parameter second data byte

  midiEventPacket_t midiMsg = {cmd >> 4, cmd, data1, data2};
  MidiUSB.sendMIDI(midiMsg);
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

//Uncomment to show vals for yAxis (-128 all down, 127 all up, 0 neutral)
//  Serial.println(n64_report.yAxis, DEC);

//Set velocity based on joystick Y val
vel = map(n64_report.yAxis, -128, 127, velFloor, 127);
//Serial.println(vel);

  
//***Drum Mode Code
if(drumMode==true){

//CC (Modulation) Control
if(alwaysUp==true){
  int ccVal = map (abs(n64_report.xAxis), -128, 127, -maxVal, maxVal); //translate value from controller input based on config settings
  //Serial.println(ccVal);
//Send the CC Message  
  //Traditional
  MIDI.sendControlChange(controlVal, ccVal, drumChT); //CC, value, channel (1-16)
  //USB
  controlChange(drumChU, controlVal, ccVal); //channel (0-15), CC, value
  MidiUSB.flush();
}else if (alwaysUp==false){
  int ccVal = map (n64_report.xAxis, -128, 127, minVal, maxVal); // translate value from controller input based on config settings
  //Serial.println(ccVal);
//Send the CC Message  
  //Traditional
  MIDI.sendControlChange(controlVal, ccVal, drumChT); //CC, value, channel (1-16)
  //USB
  controlChange(drumChU, controlVal, ccVal); //channel (0-15), CC, value
  MidiUSB.flush();
}

//A Button Trigger
  if(n64_report.a==1){
    //Serial.println ("A Pressed!");
    //If button pressed send note
    //Send on USB
   noteOn(drumChU, 48, vel);   // Channel (0-15), pitch, velo
   MidiUSB.flush();
   //Send on Traditional MIDI
   MIDI.sendNoteOn(48, vel, drumChT); // pitch, velo, channel (1-16)
 } else{
 //If button is not pressed, send note off
 //NoteOff for USB
  noteOff(drumChU, 48, vel);  // Channel (0-15), pitch, velo
  MidiUSB.flush();
 //NoteOff for Traditional MIDI
  MIDI.sendNoteOff(48, vel, drumChT); // pitch, velo, channel (1-16)
 }
}

//***Synth Mode Code
else if (synthMode==true){
//Pitch Bend for USB
//Uncomment to see X Axis vals (-128 L, 127 R)
  //Serial.println(n64_report.xAxis, DEC);
//Use X Axis to set Pitch Bend
  //Probably a smarter way to do this, but the controller range doesn't map perfectly so I've fudged it. 
  //The map range is there to return 8192 at neutral for no pitch bend as the first priority. The constrain takes that close-but-not-quite value and puts it into the true PB range as neatly as possible for my poor math skills.
int pitchBend = constrain(map (n64_report.xAxis, -127, 127, bendLow, bendHigh), bendLow, bendHigh);
// Serial.println(pitchBend);
int pitchBendUSBval = pitchBend; //make a seperate value to modify so we can pass the original value to Traditional MIDI 
//Convert 8 bit pitchbend value to 7 bit
  pitchBendUSBval = pitchBendUSBval << 1;     // shift low byte's msb to high byte
  byte msb = highByte(pitchBendUSBval);      // get the high bits
  byte lsb = lowByte(pitchBendUSBval) >> 1;  // get the low 8 bits
  //Serial.print(" ");
  //Serial.print(msb);
  //Serial.print(" ");
  //Serial.println(lsb);
 // send the pitch bend message:
  midiCommand(pbUSB, lsb, msb); //number after E is the channel for the message

//Pitch Bend for Traditional MIDI
pitchBend = pitchBend - 8192; //convert pb value for use by the Traditional MIDI Library
MIDI.sendPitchBend(pitchBend, pbT);
//Serial.println(pitchBend);
  
  //A Button Trigger
  if(n64_report.a==1){
    //Serial.println ("A Pressed!");
    //If button pressed send note
    //Send on USB
   noteOn(synthChU, 60, vel);   // Channel (0-15), pitch, velo
   MidiUSB.flush();
   //Send on Traditional MIDI
   MIDI.sendNoteOn(60, vel, synthChT); // pitch, velo, channel (1-16)
 } else{
 //If button is not pressed, send note off
 //NoteOff for USB
  noteOff(synthChU, 60, vel);  // Channel (0-15), pitch, velo
  MidiUSB.flush();
 //NoteOff for Traditional MIDI
  MIDI.sendNoteOff(60, vel, synthChT); // pitch, velo, channel (1-16)
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
