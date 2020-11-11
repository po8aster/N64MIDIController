# N64>MIDI
## N64 MIDI Controller

N64>MIDI is an Arduino-based interface that takes input from any standard N64 Controller, translates those inputs to MIDI notes and CC data, and transmits them via both USB and 5-pin MIDI.

N64>MIDI has two modes, a Drum Mode and a Synth Mode loosely based on the ocarina controls from Ocarina of Time. See the [Controller Mapping](https://github.com/po8aster/N64MIDIController/tree/master/Controller%20Mapping) section for details on how each mode functions. Pressing the button on the interface switches between modes, indicated by the LEDs.

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
  * This project uses the same hardware as the [Gamecube MIDI Interface](https://github.com/po8aster/GCMIDIController). The only difference is that the controller connections for the N64 are only 3.3v, GND, and Data. The other controller connections in the schematic are not used.
* Parts list:
  * Arduino Pro Micro - 5v Version
  * 220 Ohm Resistor x3 (1/4 watt is fine)
  * Bi-Directional Logic Level Converter (it doesn't actually have to be bi-directional, but that's what's in my wiring diagram so just go with this if you don't know the different wiring for a unidirectional one)
  * 5v to 3.3v Voltage Regulator Module (this module contains all the capacitors etc for voltage conversion. You can of course create your own, but these things are dirt cheap so if you aren't sure how to do that I'd just grab the little module)
  * LEDs x2 (choose whatever color you want, but for what it's worth I use Green for Drum Mode and Blue for Synth Mode)
  * Momentary Pushbutton
  * MIDI (5 Pin DIN) Connector
  * N64 Controller Extension Cable
* For more details on hooking things up, take a look at NicoHood's library linked above. My project simply modifies that project to convert inputs to MIDI signals on the code side, so reading over those guides is well worth your time.

  
#### If you build it and like it, please consider a small donation using the Sponsor button to buy me a beer for my efforts. Obviously not required at all, but always appreciated! 

## 5v YOLO Version
I created my prototype using a 5v Pro Micro Board without using a Logic Level Converter or Voltage Regulator. If you are really adverse to getting those components, read on.

The N64 Controller is only rated for 3.3v, so **using a 5v board could destroy your controller**. You should only do this if you don't care if the controller you plug in is destroyed. I was using a cheap eBay knockoff and didn't have issues, but I wouldn't have been too upset if it had melted.

But for the sake of knowledge: if you remove the Logic Level Converter and Voltage Regulator from the schematic it will still function, but don't be upset with me if it destroys your controller. I highly reccommend just spending the extra $1-2.
