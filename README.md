# N64>MIDI
## N64 MIDI Controller

N64>MIDI is an Arduino-based interface that takes input from any standard N64 Controller, translates those inputs to MIDI notes and CC data, and transmits them via both USB and 5-pin MIDI.

N64>MIDI has two modes, a Drum Mode and a Synth Mode loosely based on the ocarina controls from Ocarina of Time. See the [Controller Mapping](https://github.com/po8aster/N64MIDIController/tree/master/Controller%20Mapping) section for details on how each mode functions. Pressing the button switches between modes, indicated by the LEDs.

## Software:
* See the [Code](https://github.com/po8aster/N64MIDIController/tree/master/Code) section to download or copy the code.
  * If you aren't familiar with Ardunio IDE, loading sketches, and loading libraries there are lots of great tutorials out there, and I won't try to recreate that info here. But Google those terms and you should be on your way!
* Required Libraries:
  * [MIDI Library - Francois Best](https://www.arduino.cc/reference/en/libraries/midi-library/)
  * [USB MIDI Library - Gary Grewal](https://github.com/arduino-libraries/MIDIUSB)
  * [Nintendo Library - NicoHood](https://github.com/NicoHood/Nintendo)
* If you want to change the USB MIDI Device Name after you've got it working, there is an excellent guide [here](http://liveelectronics.musinou.net/MIDIdeviceName.php) on how to do so. I am not quite sure how to credit the creator properly, but it was extremely helpful for me.

## Hardware:
* See the [Schematic](https://github.com/po8aster/N64MIDIController/tree/master/Schematic) section for wiring diagram.
* Parts list:
  * Arduino Pro Micro - 3.3v Version
  * 220 Ohm Resistor x2 (1/4 watt is fine)
  * 33 Ohm Resistor (1/2 watt)
  * LEDs x2 (choose whatever color you want, but for what it's worth I use Green for Drum Mode and Blue for Synth Mode)
  * Momentary Pushbutton
  * MIDI (5 Pin DIN) Connector
  * N64 Controller Extension Cable
* For more details on hooking things up, take a look at NicoHood's library linked above. My project simply modifies that project to convert inputs to MIDI signals on the code side, so reading over those guides is well worth your time.
  
#### If you build it and like it, please consider a small donation using the Sponsor button to buy me a beer for my efforts. Obviously not required at all, but always appreciated! 

## 5v YOLO Version
I created my prototype using a 5v Pro Micro Board as I don't have a 3.3v board handy. These are the notes if you don't have a 3.3v board and cant be bothered to get the right voltage.

The N64 Controller is only rated for 3.3v, so **using a 5v board could destroy your controller**. You should only do this if you  don't care if the controller you plug in is destroyed. I was using a cheap ebay knockoff and didn't have issues, but wouldn't have been too upset if it had melted.

But for the sake of knowledge: follow the same schematic but swap out the 33 Ohm Resistor for another 220 Ohm Resistor (1/4 will work with the 5v version). That's it, good luck!
