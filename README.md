# MPlayer
MPlayer for Funkey-S

A native video player for FunKey-OS. Big thanks go to gameblabla, the FunKey team, and gnarface from the #mplayer IRC channel - without them, this would've been a whole lot harder. It currently detects .avi, .mp4, .mkv, .webm, and .mpeg files. It can play video files larger than 240x240, but going too high will result in stuttering and poor performance.

Controls:

A - Play/Pause

B - Toggle OSD

Menu - Quit

Build instructions:

- Download the "build" folder from this repository.
- Create a symbolic link to liblzo2.so.2.0.0 called liblzo2.so.2 and make sure it's in the build directory.
- Use the OPK Creator (https://github.com/Harteex/OpenPackageCreator/releases) or mksquashfs from the command line to build an OPK file from the build directory.
- Add the resulting OPK to your FunKey-S.
- Enjoy! (Don't forget to have some video files to play first!)
