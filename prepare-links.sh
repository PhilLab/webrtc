#!/bin/bash

#set -e
echo
echo Preparing symbolic links for webrtc...
echo

preparelink()
{
	if [ ! -d "$2" ]; then
		echo ERROR: Link destination is not found \"$2\" !
		exit -1
	fi
	if [ ! -h "$1" ]; then
		echo Creating webrtc symbolic link \"$1\"...
		ln -s $2 $1
	fi
}

preparelink "build" "../webrtc-deps/webrtc-build/"


echo
echo WebRTC ready.
echo
