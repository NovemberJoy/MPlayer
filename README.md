# MPlayer
MPlayer for Funkey-S

A native video player for FunKey-OS. Big thanks go to gameblabla, the FunKey team, and gnarface from the #mplayer IRC channel - without them, this would've been a whole lot harder. It currently detects .avi, .mp4, .mkv, .webm, and .mpeg files. It can play video files larger than 240x240, but going too high will result in stuttering and poor performance.

Important note about resuming: If you want to make sure it resumes properly, close MPlayer with the Menu button while it's playing. If it's paused or if you close MPlayer by shutting the device and turning it off, the resume file is broken. This may be fixed later if I can figure out how.

Controls:

A - Play/Pause
B - Toggle OSD
Left/Right - Seek forward and backward. Hold FN to seek more.
Up/Down - Increase and decrease volume.
Fn+Down - Stretch video if it's not already 1:1 ratio
Fn+L/Fn+R - Speed up/slow down video.
X/Y - Increase and decrease subtitle size.
Start - Display a helpful tip about taking screenshots.
Menu - Close MPlayer.

Build instructions:

- Download the "build" folder from this repository.
- (If running FunKey-OS 2.0.1 or earlier) Create a symbolic link to liblzo2.so.2.0.0 called liblzo2.so.2 and make sure it's in the build directory.
- Use the OPK Creator (https://github.com/Harteex/OpenPackageCreator/releases) or mksquashfs from the terminal to build an OPK file from the build directory.
- Add the resulting OPK to your FunKey-S.
- Enjoy! (Don't forget to have some video files to play first!)
