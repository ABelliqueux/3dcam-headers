# 3dcam PSX engine

This a WIP PSX 3d engine. Use this with the [companion blender exporter](https://github.com/ABelliqueux/blender_io_export_psx_mesh) to create levels for the engine.

![3d scene](https://wiki.arthus.net/assets/3d.gif)
![pre-rendered BGs](https://wiki.arthus.net/assets/precalc.gif)
![Push things](https://wiki.arthus.net/assets/push.gif)
![Sprite](https://wiki.arthus.net/assets/sprite.gif)
![Vertex animation](https://wiki.arthus.net/assets/vertexanim.gif)

More video samples [here.](https://tube.fdn.fr/video-channels/psxdev/videos)

## Features

**Be warned this is WIP** !

![comparison](https://wiki.arthus.net/assets/rt-8b-4b.gif)  
Real-time 3D / 8bpp background / 4bpp background
  
### "Engine"

  * UV textured models
  * Vertex painted models
  * Multiple camera modes
  * Vertex animations
  * Up to 3 light sources
  * Use pre-rendered backgrounds (8bpp and 4bpp)
  * Basic collisions
  * Sound effects (VAG/XA)

### Demo Controls

 * L1, L2 : rotate light matrix.
 * R1     : Change camera mode.
 * R3 (Dualshock) : Rotate camera in orbital mode.
 * Up, Down, Left Right, L3 (Dualshock) : Move actor.
 * X      : "Jump" .
 * Select : Switch level.
  
## Planned

  * Fix and improve all the things !

# Compiling

You need to install [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) and the [Nugget+PsyQ SDK](https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/newtree#installation) before
you can build the engine. Put `mkpsxiso` and `pcsx-redux` in your $PATH and you should be good to go.

  1. Clone this repo in your nugget+PsyQ folder as a new project :
```bash
git clone https://github.com/ABelliqueux/3dcam-headers --recursive my-project
```
  2. Navigate to that folder in a terminal :
```bash
cd my-project
```
  3. Type `./isotest.sh`. This should compile the example, build an iso with `mkpsxiso` and launch it with `pcsx-redux`.  
  If you'd rather do things manually or are using Windows, this script does 3 things:  
    * `make` : compile the project  
    * `mkpsxiso -y config/OverlayExample.xml` : create the bin/cue disk image  
    * `pcsx-redux -run -iso OverlayExample.cue` : run the disk image in the [pcsx-redux](https://github.com/grumpycoders/pcsx-redux/) emulator.  
  
  On first launch, `pcsx-redux` will ask for a PSX bios. You can use your own or [the open source OpenBios](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/openbios).  
  A [prebuilt binary](https://psx.arthus.net/roms/bios/openbios.bin) is available here for convenience.  
  Set it in pcsx-redux ; `Configuration > Emulation`, then reboot the emulator ; `File > Reboot`. 
  
  4. Install the [blender extension](https://github.com/ABelliqueux/blender_io_export_psx_mesh) to create your own levels.

# Trying on real HW

If you have a real PSX, a cart flashed with [Unirom](https://github.com/JonathanDotCel/unirom8_bootdisc_and_firmware_for_ps1) and a [Serial/USB cable](https://unirom.github.io/serial_psx_cable/), you can upload the demo to the PSX memory with [NOTpsxserial](https://github.com/JonathanDotCel/NOTPSXSerial).  

The engine can use [overlays](https://github.com/JaberwockySeamonstah/PSXOverlayExample/) to load data in the psx memory, so as opposed to a 'classic' project where you can just load the psx-exe in ram, we first have to load the data to a specific address, then load the exe.  

First, comment out line 28 in `main.c`, so that the PSX won't look for the data on a CD :
```c
// #define USECD
```
The address we have to load the data to is defined by the 'load_all_overlays_here' symbol in `main.map`.  
The provided `ovly_upload_helper.sh` bash script takes care of finding that address depending on the ps-exe name.  
Thus, to load `Overlay.lvl1` and `main.ps-exe` in the psx ram, use :  
```bash
./ovly_upload_helper.sh Overlay.lvl1 main.ps-exe /dev/ttyUSB0
```

# Credits

## Sound credits

### XA files :

Lobby Time by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/3986-lobby-time
License: https://filmmusic.io/standard-license

Pixelland by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/4222-pixelland
License: https://filmmusic.io/standard-license

### VAG files

All the sound effects come from https://www.myinstants.com and are :  

```
comedy_pop_finger_in_mouth_001(1).mp3
cuek.swf.mp3
erro.mp3
m4a1_single-kibblesbob-8540445.mp3
punch.mp3
wrong-answer-sound-effect.mp3
yooooooooooooooooooooooooo_4.mp3
hehehehhehehehhehehheheehehe.mp3
```

PSX code based on [example](https://psx.arthus.net/code/primdraw.7z) by [Lameguy64](https://github.com/Lameguy64)  
An incredible amount of help from the good fellows at the [psxdev discord](https://discord.com/invite/EnaNgrqJ?utm_source=Discord%20Widget&utm_medium=Connect),  
Including but not limited to @NicolasNoble, @Lameguy64, @Impiaa, @paul, @sickle, @danhans42...
