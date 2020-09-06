# N64MIDIController
N64 MIDI Controller
Code and Mapping Documentation Repository for N64 MIDI Controller Project.

Project to create an Arduino-based adapter that takes input from any standard N64 Controller and translates those inputs to MIDI notes and CC data.

Requirements:

-Compatible with original or 3rd party hardware controllers, without adapting or damaging the controller

-Drum mode following the General MIDI standard Drum Map on Channel 10

-Synth mode (loosely) based on the Ocarina controls from Ocarina of Time on Channel 1

-Send MIDI out on both USB and 5 Pin MIDI if possible, create alternate code for either if not


Notes on  current Prototype:
I am currently prototyping with a 5v Pro Micro Board as I don't have a 3.3v board handy (YOLO).

The N64 Controller is only rated for 3.3v. So while I have not had any issues running 5v, I am using a cheap knockoff controller for testing and wouldn't be too heartbroken if I killed it. Please don't connect a controller you care about to a 5v board.

I have listed the 3.3v version on my schematic as I'd rather have my schematic just not work than break people's stuff if they TL;DR these notes.
The resistor values on the schematic are the ones tested and working with a 5v board. They might work with the 3.3v, but will most likely change. Again this is not tested but if you have a 3.3v board and wanna give it a go, theoretically they would change to:
     - R1 (MIDI Pin 4 to VCC) - 33R @ .5 Watts (this seems like a weird value, purely basing this off a 3.3v MIDI Spec I dug up so this is the one I am most concerned with       
                                  testing. I expect a more common 47R will work fine though)
     - R2 & R3 (LED Resistors) - 100R (.25 watt is fine here) I honestly don't know that you'll notice a difference with between 100 and 220R here, but maybe a bit brighter.
