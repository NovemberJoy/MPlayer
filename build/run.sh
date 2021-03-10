#!/bin/sh

if [ ! -d "$HOME/.mplayer" ]; then #Make directory if it doesn't exist
mkdir $HOME/.mplayer
fi

cp -f input.conf config subfont.ttf $HOME/.mplayer #Use the included input file.
if [ ! -f "$HOME/.mplayer/setupcomplete" ]; then
rw
cp -f liblzo2.so.2 liblzo2.so.2.0.0 /usr/lib #Can we copy necessary libraries using a shell script? I really want to be able to package this as an OPK.
touch $HOME/.mplayer/setupcomplete
ro
fi #We will probably need more libraries than this.
#cp config $HOME/.mplayer/config #Use the included config file.
#cp subfont.ttf $HOME/.mplayer/subfont.ttf #Use the included TTF font.

./mplayer -vf scale -x 240 -y 240 "$1" #Try to run mplayer with the selected file?
