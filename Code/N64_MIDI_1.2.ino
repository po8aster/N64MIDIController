/*
  N64 MIDI Controller v1.1
  By Po8aster
  Based on the N64Controller example from the Nintendo library by NicoHood

  Required Libraries (as named in Library Manager):
  Nintendo by NicoHood
  MIDI Library by Francois Best
  MIDIUSB by Gary Grewal
*/
//----------------------------------------------------------------
//Include the Nintendo Library
#include "Nintendo.h"
//Include USB and Traditional MIDI Libraries
#include "MIDIUSB.h"
#include "MIDI.h"
#include <pitchToNote.h>  // include pitch chart from MIDIUSB library

//This makes the traditional MIDI Work
MIDI_CREATE_DEFAULT_INSTANCE();
//----------------------------------------------------------------
//*** Definitions to Update Settings/Channels/Etc
//Intended to be easily modifiable to customize the controller to your needs/preferences without having to get too deep in my awful code

// Define a N64 Controller and the pin for data connection
CN64Controller N64Controller(7); //change this number to match pin the controller data line is connected to (only change if you built yours differently than my schematic)

// Pin definitions (only change these if you built yours differently than my schematic)
int drumLED = 4; // pin for the drum mode LED
int synthLED = 5; // pin for the synth mode LED
int buttonPin = 10; // pin for the mode change button

//MIDI Channel Definitions (defaults to Channel 1 for Synth and Channel 10 for Drums on both USB and Traditional MIDI but can be changed as desired)
int drumChU = 9; //channel for USB in Drum Mode (0-15)
int drumChT = 10; //channel for Traditional in Drum Mode (1-16)
int synthChU = 0; //channel for USB in Synth Mode (0-15)
int synthChT = 1; //channel for Traditional in Synth Mode (1-16)
byte pbUSB = 0xE0; // set last digit to channel (0-15) for pitch bend messages via USB (match synthChU in most cases)ie 0xE1 for Channel 2
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

//Pitch Bend Definitions (set high and low limits for pitch bend, 4096/12288 = 1/2 wheel range, 1/16383 is max wheel range)
int bendLow = 4096;
int bendHigh = 12288;

//Octave Limits (defaults to two octaves up or down from middle C in synth mode but can be adjusted as desired)
int octaveMin = -2; //number of octaves down allowed
int octaveMax = 2; //number of octaves up allowed

//***Button to Note Mapping 
//Change number to correspond to desited MIDI Note (ie Middle C = 60). Z holds in synth mode will be 1 half-step down from whatever you map the root button to.

//Drum Mode
int drumA = 40; //snare
int drumB = 38; //snare 2
int drumL = 35; //kick
int drumR = 42; //closed hat
int drumZ = 36; //kick 2
int drumStart = 52; //chinese

//D Pad
int drumDUp = 56; //cowbell
int drumDUpR = 50; //hi tom
int drumDR = 48; //hi-mid tom
int drumDDownR = 47; //low-mid tom
int drumDDown = 45; //low tom
int drumDDownL = 43; //hi-floor tom
int drumDL = 41; //low-floor tom
int drumDUpL = 54; //tambourine

//C Buttons
int drumCUp = 44; //pedal hat
int drumCUpR = 51; //ride
int drumCR = 46; //open hat
int drumCDownR = 55; //splash
int drumCDown = 39; //clap
int drumCDownL = 53; //ride bell
int drumCL = 37; //side stick
int drumCUpL = 49; //crash


//Synth Mode
int synthA = 69; //A
int synthB = 71; //B
int synthL = 42; //Closed Hat on Drum Channel
int synthR = 65; //F

//C Buttons
int synthCDown = 60; //C
int synthCL = 62; //D
int synthCUp = 64; //E
int synthCR = 67; //G

//D Pad
int synthDR = 38; //Snare 2 on Drum Channel
int synthDL = 36; //Kick 2 on Drum Channel

//----------------------------------------------------------------
// *** Here Be Dragons, the below is not intended to be easily modified but is where you will go to change how the thing works instead of simple tweaks

//Octave Control Definitions
int octave = 0; //initial octave (0 = MIDI 4, starting at Middle C)
int octaveUp = 0; // used to hold octave change value
int octaveDown = 0;
int octaveUpLast = 0; // used to detect change in octave
int octaveDownLast = 0;

//Mode bool definitions
bool drumMode = true;
bool synthMode = false;
bool timeout = false;
int timeoutLength = 500; //set the delay time for mode switch timeout


//State tracking for Mod/CC, Reset, Mode Switch, and Pitch Bend
int ccState = 0;
int ccStateLast = 0;
int resetState = 0;
int resetStateLast = 0;
int modeState = 0;
int modeStateLast = 0;
int pbState = 0;
int pbStateLast = 0;

//State Tracking for notes
//Drum Mode
int drumAState = 0; //snare
int drumAStateLast = 0; //snare
int drumBState = 0;
int drumBStateLast = 0;
int drumLState = 0;
int drumLStateLast = 0;
int drumRState = 0;
int drumRStateLast = 0;
int drumZState = 0;
int drumZStateLast = 0;
int drumStartState = 0;
int drumStartStateLast = 0;

//D Pad
int drumDUpState = 0;
int drumDUpStateLast = 0;
int drumDUpRState = 0;
int drumDUpRStateLast = 0;
int drumDRState = 0;
int drumDRStateLast = 0;
int drumDDownRState = 0;
int drumDDownRStateLast = 0;
int drumDDownState = 0;
int drumDDownStateLast = 0;
int drumDDownLState = 0;
int drumDDownLStateLast = 0;
int drumDLState = 0;
int drumDLStateLast = 0;
int drumDUpLState = 0;
int drumDUpLStateLast = 0;

//C Buttons
int drumCUpState = 0;
int drumCUpStateLast = 0;
int drumCUpRState = 0;
int drumCUpRStateLast = 0;
int drumCRState = 0;
int drumCRStateLast = 0;
int drumCDownRState = 0;
int drumCDownRStateLast = 0;
int drumCDownState = 0;
int drumCDownStateLast = 0;
int drumCDownLState = 0;
int drumCDownLStateLast = 0;
int drumCLState = 0;
int drumCLStateLast = 0;
int drumCUpLState = 0;
int drumCUpLStateLast = 0;


//Synth Mode
int synthAState = 0;
int synthAStateLast = 0;
int synthAZState = 0;
int synthAZStateLast = 0;
int synthBState = 0;
int synthBStateLast = 0;
int synthBZState = 0;
int synthBZStateLast = 0;
int synthLState = 0;
int synthLStateLast = 0;
int synthRState = 0;
int synthRStateLast = 0;
int synthRZState = 0;
int synthRZStateLast = 0;

//C Buttons
int synthCDownState = 0;
int synthCDownStateLast = 0;
int synthCDownZState = 0;
int synthCDownZStateLast = 0;
int synthCLState = 0;
int synthCLStateLast = 0;
int synthCLZState = 0;
int synthCLZStateLast = 0;
int synthCUpState = 0;
int synthCUpStateLast = 0;
int synthCUpZState = 0;
int synthCUpZStateLast = 0;
int synthCRState = 0;
int synthCRStateLast = 0;
int synthCRZState = 0;
int synthCRZStateLast = 0;
//D Pad
int synthDRState = 0;
int synthDRStateLast = 0;
int synthDLState = 0;
int synthDLStateLast = 0;


//----------------------------------------------------------------
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

//----------------------------------------------------------------
//Mode Toggle Function
void toggleMode(){
  //set timeout
  timeout = true;
  
  // trigger all notes off
  allNotesOff();
  
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
  //wait then toggle timeout
  delay(timeoutLength);
  timeout = false;
}

//All Notes Off on all channels
void allNotesOff(){
  //Send all notes off (CC 123) to prevent held notes
  //Sends to all channels for all modes

  //Drum Mode
  //Traditional
  MIDI.sendControlChange(123, 0, drumChT); //CC, value, channel (1-16)
  //USB
  controlChange(drumChU, 123, 0); //channel (0-15), CC, value
  MidiUSB.flush();

  //Synth Mode
  //Traditional
  MIDI.sendControlChange(123,0,synthChT); //CC, value, channel (1-16)
  //USB
  controlChange(synthChU, 123, 0); //channel (0-15), CC, value
  MidiUSB.flush();
}

//----------------------------------------------------------------
//Functions for sending Drum and Synth Notes
void synthNoteOn(int note, int octave){ 
    //Send on USB
    noteOn(synthChU, note + (octave*12), vel);   // Channel (0-15), pitch, velo
    MidiUSB.flush();
    //Send on Traditional MIDI
    MIDI.sendNoteOn(note + (octave*12), vel, synthChT); // pitch, velo, channel (1-16)
  }

void drumNoteOn(int note){ 
    //Send on USB
    noteOn(drumChU, note, vel);   // Channel (0-15), pitch, velo
    MidiUSB.flush();
    //Send on Traditional MIDI
    MIDI.sendNoteOn(note, vel, drumChT); // pitch, velo, channel (1-16)
  }

void synthNoteOff(int note, int octave){
    // NoteOff for USB
    noteOff(synthChU, note + (octave*12), vel);  // Channel (0-15), pitch, velo
    MidiUSB.flush();
    // NoteOff for Traditional MIDI
    MIDI.sendNoteOff(note + (octave*12), vel, synthChT); // pitch, velo, channel (1-16)
 }

void drumNoteOff(int note){
    // NoteOff for USB
    noteOff(drumChU, note, vel);  // Channel (0-15), pitch, velo
    MidiUSB.flush();
    // NoteOff for Traditional MIDI
    MIDI.sendNoteOff(note, vel, drumChT); // pitch, velo, channel (1-16)
 }
 
//----------------------------------------------------------------
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
  MIDI.begin(synthChT);

  //Set the default LED
  digitalWrite(drumLED, HIGH);
}

//----------------------------------------------------------------
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
  modeState = digitalRead(buttonPin);
  //if button state has changed, switch mode
  if(modeState != modeStateLast){
  //Serial.println(buttonVal);
  //If button pressed (0) and timeout done switch mode
  if(modeState==0 && timeout == false){
    toggleMode();
  }
}
//update last state
modeStateLast = modeState;

//Uncomment to show vals for yAxis (-128 all down, 127 all up, 0 neutral)
//  Serial.println(n64_report.yAxis, DEC);

//Set velocity based on joystick Y val
vel = map(n64_report.yAxis, -128, 127, velFloor, 127);
//Serial.println(vel);

//----------------------------------------------------------------
//***Drum Mode Code
if(drumMode==true){

//CC (Modulation) Control
//check if CC Value has changed, if so, calculate and send CC value
ccState = n64_report.xAxis;
if(ccState != ccStateLast){
if(alwaysUp==true){
  int ccVal = map (abs(n64_report.xAxis), -128, 127, -maxVal, maxVal); //translate value from controller input based on config settings
//Send the CC Message  
  //Traditional
  MIDI.sendControlChange(controlVal, ccVal, drumChT); //CC, value, channel (1-16)
  //USB
  controlChange(drumChU, controlVal, ccVal); //channel (0-15), CC, value
  MidiUSB.flush();
}else if (alwaysUp==false){
  int ccVal = map (n64_report.xAxis, -128, 127, minVal, maxVal); // translate value from controller input based on config settings
//Send the CC Message  
  //Traditional
  MIDI.sendControlChange(controlVal, ccVal, drumChT); //CC, value, channel (1-16)
  //USB
  controlChange(drumChU, controlVal, ccVal); //channel (0-15), CC, value
  MidiUSB.flush();
}
}
//update CC Last State
ccStateLast = ccState;

//**** Begin Drum Note Triggers

//Set states based on buttons
drumAState = n64_report.a;
drumBState = n64_report.b;
drumLState = n64_report.l;
drumRState = n64_report.r;
drumZState = n64_report.z;
drumStartState = n64_report.start;

//Set D Pad states
   //DPad Up
    if(n64_report.dup == 1 && n64_report.dleft == 0 && n64_report.dright == 0){
      drumDUpState = 1;
    } else{
      drumDUpState = 0;
    }
   //DPad Up + L
    if(n64_report.dup == 1 && n64_report.dleft == 1 && n64_report.dright == 0){
      drumDUpLState = 1;
    } else{
      drumDUpLState = 0;
    }
  //DPad Up + R
    if(n64_report.dup == 1 && n64_report.dleft == 0 && n64_report.dright == 1){
      drumDUpRState = 1;
    } else{
      drumDUpRState = 0;
    }
   //DPad Down
    if(n64_report.ddown == 1 && n64_report.dleft == 0 && n64_report.dright == 0){
      drumDDownState = 1;
    } else{
      drumDDownState = 0;
    }
  //DPad Down + L
    if(n64_report.ddown == 1 && n64_report.dleft == 1 && n64_report.dright == 0){
      drumDDownLState = 1;
    } else{
      drumDDownLState = 0;
    }
  //DPad Down + R
    if(n64_report.ddown == 1 && n64_report.dleft == 0 && n64_report.dright == 1){
      drumDDownRState = 1;
    } else{
      drumDDownRState = 0;
    }
  //DPad Left
    if(n64_report.ddown == 0 && n64_report.dleft == 1 && n64_report.dup == 0){
      drumDLState = 1;
    } else{
      drumDLState = 0;
    }
  //DPad Right
    if(n64_report.ddown == 0 && n64_report.dright == 1 && n64_report.dup == 0){
      drumDRState = 1;
    } else{
      drumDRState = 0;
    }
  
  //Set C Button states
   //C Pad Up
    if(n64_report.cup == 1 && n64_report.cleft == 0 && n64_report.cright == 0){
      drumCUpState = 1;
    } else{
      drumCUpState = 0;
    }
   //C Pad Up + L
    if(n64_report.cup == 1 && n64_report.cleft == 1 && n64_report.cright == 0){
      drumCUpLState = 1;
    } else{
      drumCUpLState = 0;
    }
  //C Pad Up + R
    if(n64_report.cup == 1 && n64_report.cleft == 0 && n64_report.cright == 1){
      drumCUpRState = 1;
    } else{
      drumCUpRState = 0;
    }
   //C Pad Down
    if(n64_report.cdown == 1 && n64_report.cleft == 0 && n64_report.cright == 0){
      drumCDownState = 1;
    } else{
      drumCDownState = 0;
    }
  //C Pad Down + L
    if(n64_report.cdown == 1 && n64_report.cleft == 1 && n64_report.cright == 0){
      drumCDownLState = 1;
    } else{
      drumCDownLState = 0;
    }
  //C Pad Down + R
    if(n64_report.cdown == 1 && n64_report.cleft == 0 && n64_report.cright == 1){
      drumCDownRState = 1;
    } else{
      drumCDownRState = 0;
    }
  //C Pad Left
    if(n64_report.cdown == 0 && n64_report.cleft == 1 && n64_report.cup == 0){
      drumCLState = 1;
    } else{
      drumCLState = 0;
    }
  //C Pad Right
    if(n64_report.cdown == 0 && n64_report.cright == 1 && n64_report.cup == 0){
      drumCRState = 1;
    } else{
      drumCRState = 0;
    }

//*** Note Triggers    
//A Button
  //If state has changed start or stop note
  if(drumAState != drumAStateLast){
    //Send the note if changed to on
    if(drumAState==1){
    drumNoteOn(drumA);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumA);
  }
 }

//B Button
  //If state has changed start or stop note
  if(drumBState != drumBStateLast){
    //Send the note if changed to on
    if(drumBState==1){
    drumNoteOn(drumB);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumB);
  }
 }

//L Button
  //If state has changed start or stop note
  if(drumLState != drumLStateLast){
    //Send the note if changed to on
    if(drumLState==1){
    drumNoteOn(drumL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumL);
  }
 }

//R Button
  //If state has changed start or stop note
  if(drumRState != drumRStateLast){
    //Send the note if changed to on
    if(drumRState==1){
    drumNoteOn(drumR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumR);
  }
 }

//Z Button
  //If state has changed start or stop note
  if(drumZState != drumZStateLast){
    //Send the note if changed to on
    if(drumZState==1){
    drumNoteOn(drumZ);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumZ);
  }
 }

//Start Button
  //If state has changed start or stop note
  if(drumStartState != drumStartStateLast){
    //Send the note if changed to on
    if(drumStartState==1){
    drumNoteOn(drumStart);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumStart);
  }
 }

//DPad Up 
  //If state has changed start or stop note
  if(drumDUpState != drumDUpStateLast){
    //Send the note if changed to on
    if(drumDUpState==1){
    drumNoteOn(drumDUp);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDUp);
  }
 }

//DPad Up + R 
  //If state has changed start or stop note
  if(drumDUpRState != drumDUpRStateLast){
    //Send the note if changed to on
    if(drumDUpRState==1){
    drumNoteOn(drumDUpR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDUpR);
  }
 }

//DPad Up + L 
  //If state has changed start or stop note
  if(drumDUpLState != drumDUpLStateLast){
    //Send the note if changed to on
    if(drumDUpLState==1){
    drumNoteOn(drumDUpL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDUpL);
  }
 }

//DPad Down 
  //If state has changed start or stop note
  if(drumDDownState != drumDDownStateLast){
    //Send the note if changed to on
    if(drumDDownState==1){
    drumNoteOn(drumDDown);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDDown);
  }
 }

//DPad Down + R
  //If state has changed start or stop note
  if(drumDDownRState != drumDDownRStateLast){
    //Send the note if changed to on
    if(drumDDownRState==1){
    drumNoteOn(drumDDownR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDDownR);
  }
 }

//DPad Down + L
  //If state has changed start or stop note
  if(drumDDownLState != drumDDownLStateLast){
    //Send the note if changed to on
    if(drumDDownLState==1){
    drumNoteOn(drumDDownL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDDownL);
  }
 }

//DPad Right
  //If state has changed start or stop note
  if(drumDRState != drumDRStateLast){
    //Send the note if changed to on
    if(drumDRState==1){
    drumNoteOn(drumDR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDR);
  }
 }

//DPad Left
  //If state has changed start or stop note
  if(drumDLState != drumDLStateLast){
    //Send the note if changed to on
    if(drumDLState==1){
    drumNoteOn(drumDL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumDL);
  }
 }

//C Pad Up 
  //If state has changed start or stop note
  if(drumCUpState != drumCUpStateLast){
    //Send the note if changed to on
    if(drumCUpState==1){
    drumNoteOn(drumCUp);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCUp);
  }
 }

//C Pad Up + R 
  //If state has changed start or stop note
  if(drumCUpRState != drumCUpRStateLast){
    //Send the note if changed to on
    if(drumCUpRState==1){
    drumNoteOn(drumCUpR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCUpR);
  }
 }

//C Pad Up + L 
  //If state has changed start or stop note
  if(drumCUpLState != drumCUpLStateLast){
    //Send the note if changed to on
    if(drumCUpLState==1){
    drumNoteOn(drumCUpL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCUpL);
  }
 }

//C Pad Down 
  //If state has changed start or stop note
  if(drumCDownState != drumCDownStateLast){
    //Send the note if changed to on
    if(drumCDownState==1){
    drumNoteOn(drumCDown);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCDown);
  }
 }

//C Pad Down + R
  //If state has changed start or stop note
  if(drumCDownRState != drumCDownRStateLast){
    //Send the note if changed to on
    if(drumCDownRState==1){
    drumNoteOn(drumCDownR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCDownR);
  }
 }

//C Pad Down + L
  //If state has changed start or stop note
  if(drumCDownLState != drumCDownLStateLast){
    //Send the note if changed to on
    if(drumCDownLState==1){
    drumNoteOn(drumCDownL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCDownL);
  }
 }

//C Pad Right
  //If state has changed start or stop note
  if(drumCRState != drumCRStateLast){
    //Send the note if changed to on
    if(drumCRState==1){
    drumNoteOn(drumCR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCR);
  }
 }

//C Pad Left
  //If state has changed start or stop note
  if(drumCLState != drumCLStateLast){
    //Send the note if changed to on
    if(drumCLState==1){
    drumNoteOn(drumCL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(drumCL);
  }
 }

//Update Last States
drumAStateLast=drumAState;
drumBStateLast=drumBState;
drumLStateLast=drumLState;
drumRStateLast=drumRState;
drumZStateLast=drumZState;
drumStartStateLast=drumStartState;
//Update DPad States
drumDUpStateLast = drumDUpState;
drumDUpRStateLast = drumDUpRState;
drumDRStateLast = drumDRState;
drumDDownRStateLast = drumDDownRState;
drumDDownStateLast = drumDDownState;
drumDDownLStateLast = drumDDownLState;
drumDLStateLast = drumDLState;
drumDUpLStateLast = drumDUpLState;
//Update CPad States
drumCUpStateLast = drumCUpState;
drumCUpRStateLast = drumCUpRState;
drumCRStateLast = drumCRState;
drumCDownRStateLast = drumCDownRState;
drumCDownStateLast = drumCDownState;
drumCDownLStateLast = drumCDownLState;
drumCLStateLast = drumCLState;
drumCUpLStateLast = drumCUpLState;

//*** End Drum Note Triggers

}

//----------------------------------------------------------------
//***Synth Mode Code
else if (synthMode==true){

//Pitch Bend
  //Probably a smarter way to do this, but the controller range doesn't map perfectly so I've fudged it. 
  //The map range is there to return 8192 at neutral for no pitch bend as the first priority. The constrain takes that close-but-not-quite value and puts it into the true PB range as neatly as possible for my poor math skills.
int pitchBend = constrain(map (n64_report.xAxis, -127, 127, bendLow, bendHigh), bendLow, bendHigh);
  //check if value has changed
  pbState = pitchBend;
  if (pbState != pbStateLast){
int pitchBendUSBval = pbState; //make a seperate value to modify for USB 
//Convert 8 bit pitchbend value to 7 bit
  pitchBendUSBval = pitchBendUSBval << 1;     // shift low byte's msb to high byte
  byte msb = highByte(pitchBendUSBval);      // get the high bits
  byte lsb = lowByte(pitchBendUSBval) >> 1;  // get the low 8 bits

  // send the pitch bend message:
  midiCommand(pbUSB, lsb, msb); //number after E is the channel for the message
  MidiUSB.flush();
  //Pitch Bend for Traditional MIDI
  int pitchBendTrad = pbState - 8192; //make a new value for Traditional MIDI
  MIDI.sendPitchBend(pitchBendTrad, pbT);
}
//update last state
pbStateLast = pbState;

//Octave Control
  //Octave Up
  //set to 1 if up pressed
  octaveUp = n64_report.dup;
  
  //if that has changed since the last loop, update the octave
  if (octaveUp != octaveUpLast){
    //if the change is the button being pressed, move the octave up one within the constraints
    if(octaveUp == 1){
      // stop all notes to prevent held notes in the old octave
      allNotesOff();
      // set octave up by 1
      octave = constrain((octave +1), octaveMin, octaveMax);
    }
  }
  octaveUpLast = octaveUp; //update the last state

  //Octave Down
  //set to 1 if down pressed
  octaveDown = n64_report.ddown;
  
  //if that has changed since the last loop, update the octave
  if (octaveDown != octaveDownLast){
    //if the change is the button being pressed, move the octave up one within the constraints
    if(octaveDown == 1){
      // stop all notes to prevent held notes in the old octave
      allNotesOff();
      // set octave down by 1
      octave = constrain((octave -1), octaveMin, octaveMax);
    }
  }
  octaveDownLast = octaveDown; //update the last state
  //Serial.println(octave);

//Start Button Reset octave and kill notes if pressed
  //check if button state changed
  resetState = n64_report.start;
  if(resetState != resetStateLast){
    //if pressed trigger reset
  if(resetState==1){
    //stop all notes on all mode channels
    allNotesOff();
    //reset octave value to zero
    octave=0;
  }
 }
 //update last state
 resetStateLast = resetState;
   
//**** Begin Synth Note Triggers

//Set states based on buttons
//A
if(n64_report.a == 1 && n64_report.z == 0){
  synthAState = 1;
} else{
  synthAState = 0;
}
if(n64_report.a == 1 && n64_report.z == 1){
  synthAZState = 1;
}else{
  synthAZState = 0;
}

//B
if(n64_report.b == 1 && n64_report.z == 0){
  synthBState = 1;
} else{
  synthBState = 0;
}
if(n64_report.b == 1 && n64_report.z == 1){
  synthBZState = 1;
}else{
  synthBZState = 0;
}

//L
synthLState = n64_report.l;//drum

//R
if(n64_report.r == 1 && n64_report.z == 0){
  synthRState = 1;
} else{
  synthRState = 0;
}
if(n64_report.r == 1 && n64_report.z == 1){
  synthRZState = 1;
}else{
  synthRZState = 0;
}

//C Down
if(n64_report.cdown == 1 && n64_report.z == 0){
  synthCDownState = 1;
} else{
  synthCDownState = 0;
}
if(n64_report.cdown == 1 && n64_report.z == 1){
  synthCDownZState = 1;
}else{
  synthCDownZState = 0;
}

//C Left
if(n64_report.cleft == 1 && n64_report.z == 0){
  synthCLState = 1;
} else{
  synthCLState = 0;
}
if(n64_report.cleft == 1 && n64_report.z == 1){
  synthCLZState = 1;
}else{
  synthCLZState = 0;
}

//C Up
if(n64_report.cup == 1 && n64_report.z == 0){
  synthCUpState = 1;
} else{
  synthCUpState = 0;
}
if(n64_report.cup == 1 && n64_report.z == 1){
  synthCUpZState = 1;
}else{
  synthCUpZState = 0;
}

//C Right
if(n64_report.cright == 1 && n64_report.z == 0){
  synthCRState = 1;
} else{
  synthCRState = 0;
}
if(n64_report.cright == 1 && n64_report.z == 1){
  synthCRZState = 1;
}else{
  synthCRZState = 0;
}
//D Pad
synthDRState = n64_report.dright;//drum
synthDLState = n64_report.dleft;//drum

//A Button
  //if button State changed, send on or off message for associated note
  if(synthAState != synthAStateLast){
    //send note if pressed
    if(synthAState==1){
      synthNoteOn(synthA, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthA, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthAZState != synthAZStateLast){
    //send note if pressed
    if(synthAZState==1){
      synthNoteOn(synthA -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthA -1, octave);
    }
  }

//B Button
  //if button State changed, send on or off message for associated note
  if(synthBState != synthBStateLast){
    //send note if pressed
    if(synthBState==1){
      synthNoteOn(synthB, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthB, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthBZState != synthBZStateLast){
    //send note if pressed
    if(synthBZState==1){
      synthNoteOn(synthB -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthB -1, octave);
    }
  }

//L Button
  //If state has changed start or stop note
  //Treat as drum note to send on designated drum channels
  if(synthLState != synthLStateLast){
    //Send the note if changed to on
    if(synthLState==1){
    drumNoteOn(synthL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(synthL);
  }
 }

//R Button
  //if button State changed, send on or off message for associated note
  if(synthRState != synthRStateLast){
    //send note if pressed
    if(synthRState==1){
      synthNoteOn(synthR, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthR, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthRZState != synthRZStateLast){
    //send note if pressed
    if(synthRZState==1){
      synthNoteOn(synthR -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthR -1, octave);
    }
  }

//C Down
  //if button State changed, send on or off message for associated note
  if(synthCDownState != synthCDownStateLast){
    //send note if pressed
    if(synthCDownState==1){
      synthNoteOn(synthCDown, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCDown, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthCDownZState != synthCDownZStateLast){
    //send note if pressed
    if(synthCDownZState==1){
      synthNoteOn(synthCDown -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCDown -1, octave);
    }
  }

//C Left
  //if button State changed, send on or off message for associated note
  if(synthCLState != synthCLStateLast){
    //send note if pressed
    if(synthCLState==1){
      synthNoteOn(synthCL, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCL, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthCLZState != synthCLZStateLast){
    //send note if pressed
    if(synthCLZState==1){
      synthNoteOn(synthCL -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCL -1, octave);
    }
  }

//C Up
  //if button State changed, send on or off message for associated note
  if(synthCUpState != synthCUpStateLast){
    //send note if pressed
    if(synthCUpState==1){
      synthNoteOn(synthCUp, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCUp, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthCUpZState != synthCUpZStateLast){
    //send note if pressed
    if(synthCUpZState==1){
      synthNoteOn(synthCUp -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCUp -1, octave);
    }
  }

//C Right
  //if button State changed, send on or off message for associated note
  if(synthCRState != synthCRStateLast){
    //send note if pressed
    if(synthCRState==1){
      synthNoteOn(synthCR, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCR, octave);
    }
  }
  //if Z State changed, send on or off message for associated note
  if(synthCRZState != synthCRZStateLast){
    //send note if pressed
    if(synthCRZState==1){
      synthNoteOn(synthCR -1, octave);
    }
    //note off if not
    else {
      synthNoteOff(synthCR -1, octave);
    }
  }

//D Pad Left
  //If state has changed start or stop note
  //Treat as drum note to send on designated drum channels
  if(synthDLState != synthDLStateLast){
    //Send the note if changed to on
    if(synthDLState==1){
    drumNoteOn(synthDL);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(synthDL);
  }
 }

//D Pad Right
  //If state has changed start or stop note
  //Treat as drum note to send on designated drum channels
  if(synthDRState != synthDRStateLast){
    //Send the note if changed to on
    if(synthDRState==1){
    drumNoteOn(synthDR);
  }
    //Stop the note if changed to off
    else{
    drumNoteOff(synthDR);
  }
 }

//Update Last States
synthAStateLast = synthAState;
synthBStateLast = synthBState;
synthAZStateLast = synthAZState;
synthBZStateLast = synthBZState;
synthLStateLast = synthLState;
synthRStateLast = synthRState;
synthRZStateLast = synthRZState;
synthCDownStateLast = synthCDownState;
synthCLStateLast = synthCLState;
synthCDownZStateLast = synthCDownZState;
synthCLZStateLast = synthCLZState;
synthCUpStateLast = synthCUpState;
synthCUpZStateLast = synthCUpZState;
synthCRStateLast = synthCRState;
synthCRZStateLast = synthCRZState;
synthDRStateLast = synthDRState;
synthDLStateLast = synthDLState;    
  }
}
