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

### Demo Controls

 * L1, L2 : rotate light matrix.
 * R1     : Change camera mode.
 * R3 (Dualshock) : Rotate camera in orbital mode.
 * Up, Down, Left Right, L3 (Dualshock) : Move actor.
 * X      : "Jump" .
 * Select : Switch level.
 
  
## Planned

  * Fix and improve all the things !
  * Wall collisions

# Compiling

You need to install [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) and the [pcsx-redux emulator and Nugget+PsyQ SDK](https://github.com/ABelliqueux/nolibgs_hello_worlds#setting-up-the-sdk--modern-gcc--psyq-aka-nuggetpsyq) before
you can build the engine. Put `mkpsxiso` and `pcsx-redux` in your $PATH and you should be good to go.

  1. Clone this repo in `(...)/pcsx-redux/src/mips/` as a new project :
```bash
git clone https://github.com/ABelliqueux/3dcam-headers my-project
```
  2. Navigate to that folder in a terminal :
```bash
cd /pcsx-redux/src/mips/my-project
```
  3. Type `./isotest.sh`. This should compile the example, build an iso with `mkpsxiso` and launch it with `pcsx-redux`.
  On first launch, `pcsx-redux` will ask for a PSX bios. You can use your own or [the open source OpenBios](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/openbios).  
  A [prebuilt binary](http://psx.arthus.net/roms/bios/openbios.bin) is available here for convenience.  
  Set it in pcsx-redux ; `Configuration > Emulation`, then reboot the emulator ; `File > Reboot`. 
  
  4. Install the [blender extension](https://github.com/ABelliqueux/blender_io_export_psx_mesh) to create your own levels.
  
# Credits

PSX code based on [example](http://psx.arthus.net/code/primdraw.7z) by [Lameguy64](https://github.com/Lameguy64)  
An incredible amount of help from the good fellows at the [psxdev discord](https://discord.com/invite/EnaNgrqJ?utm_source=Discord%20Widget&utm_medium=Connect),  
Including but not limited to @NicolasNoble, @Lameguy64, @Impiaa, @paul, @sickle, @danhans42...
