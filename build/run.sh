#!/bin/sh

RESUME=$(dirname "$1")/.$(basename "$1").resume #RESUME script written by steward-fu, used here because MPlayer is GPLv2+ and his script most likely is too.

if [ ! -d "$HOME/.mplayer" ]; then #Make directory if it doesn't exist
mkdir $HOME/.mplayer
fi

if [ ! -f "$HOME/.mplayer/setupcomplete" ]; then
rw
cp -f liblzo2.so.2 liblzo2.so.2.0.0 /usr/lib #Copy libraries to necessary location for versions of FunKey OS below 2.1.0.
cp -f input.conf config subfont.ttf $HOME/.mplayer #Use included config and font
touch $HOME/.mplayer/setupcomplete
ro
fi

#Following script used for resume functionality from steward-fu's Miyoo port of MPlayer. I hope to one day be this good at shell scripting.

if [ -f "$RESUME" ]; then
	NUM=$(cat -v "$RESUME" | tail -3 | head -1 | sed 's/.*\ V://' | cut -dA -f1 | xargs)
	if echo "$NUM" | egrep -q '^\-?[0-9]*\.?[0-9]+$'; then
		echo "This is correct!"
	else
		echo "Improperly-formatted RESUME file, removing!"
		rm "$RESUME"
	fi
fi

if [ -f "$RESUME" ]; then
	END=$(cat -v "$RESUME" | tail -3 | head -1 | sed 's/.*\ V://' | cut -dA -f1 | xargs) #Parses time information from the generated .resume file, but fails if the time information isn't in exactly the right place.
	./mplayer -ss $END "$1" | tee "$RESUME" #Attempt to seek to correct position.
else
    ./mplayer "$1" | tee "$RESUME" #Don't seek with RESUME if it doesn't exist.
fi

EXIT=$(tail -n 1 "$RESUME") #Get output when MPlayer exits.

if [ "$EXIT" == "Exiting... (End of file)" ]; then
	echo "End"
	rm "$RESUME"
fi
