# 3dcam PSX engine

This a WIP PSX 3d engine. Use this with the [companion blender exporter](https://github.com/ABelliqueux/blender_io_export_psx_mesh) to create levels for the engine.

![3d scene](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/3d.gif)
![pre-rendered BGs](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/precalc.gif)
![Push things](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/push.gif)
![Sprite](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/sprite.gif)
![Vertex animation](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/vertexanim.gif)

More video samples [here.](https://tube.fdn.fr/video-channels/psxdev/videos)

## Features

**Be warned this is WIP** !

![comparison](https://github.com/ABelliqueux/blender_io_export_psx_mesh/blob/main/gif/rt-8b-4b.gif)  
Real-time 3D / 8bpp background / 4bpp background
  
### "Engine"

  * Very basic physics / collision detection
  * Constrained camera trajectory
  * Orbital camera mode
  * Basic Spatial partitioning
  * Portal based camera angle switch
  * 3D sprite
  * VRam auto layout for TIMs

## Planned

  * Fix and improve all the things !
  * Wall collisions

# Compiling

You need to install [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) and the [pcsx-redux emulator and Nugget+PsyQ SDK](https://github.com/ABelliqueux/nolibgs_hello_worlds#setting-up-the-sdk--modern-gcc--psyq-aka-nuggetpsyq) before
you can build the engine. Put `mkpsxiso` and `pcsx-redux` in your $PATH and you should be good to go.

  1. Clone this repo in `(...)/pcsx-redux/src/mips/`
  2. Navigate to that folder in a terminal :
```bash
cd /pcsx-redux/src/mips/3dcam-headers
```
  3. Type `./isotest.sh`. This should compile the example, build an iso with `mkpsxiso` and launch it with `pcsx-redux`.
  4. Install the [blender extension](https://github.com/ABelliqueux/blender_io_export_psx_mesh) to create your own levels.
  
# Credits

PSX code based on [example](http://psx.arthus.net/code/primdraw.7z) by [Lameguy64](https://github.com/Lameguy64)  
An incredible amount of help from the good fellows at the [psxdev discord](https://discord.com/invite/EnaNgrqJ?utm_source=Discord%20Widget&utm_medium=Connect),  
Including but not limited to @NicolasNoble, @Lameguy64, @Impiaa, @paul, @sickle, @danhans42...