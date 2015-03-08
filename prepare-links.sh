#!/bin/bash

#set -e
echo
echo Preparing symbolic links for webrtc...
echo

preparelink()
{
	if [ ! -d "$1" ]; then
		echo ERROR: Path to link does not exist \"$1\" !
	fi
	pushd $1 > /dev/null
	if [ ! -d "$3" ]; then
		echo ERROR: Link destination is not found \"$3\" !
		popd > /dev/null
		exit -1
	fi
	if [ ! -h "$2" ]; then
		echo In path \"$1\" creating webrtc symbolic link \"$2\" pointing to \"$3\"...
		ln -s $3 $2
		if [ $? -ne 0 ]; then
			failure=$?
			echo Faield to create symbolic link
			popd > /dev/null
			exit $failure
		fi
	fi
	popd > /dev/null
}

preparelink "." "build" "../webrtc-deps/webrtc-build/"
preparelink "third_party/yasm/source" "patched-yasm" "../../../../webrtc-deps/patched-yasm/"
preparelink "third_party/opus" "src" "../../../webrtc-deps/opus/"
preparelink "third_party" "libsrtp" "../../webrtc-deps/libsrtp/"
preparelink "third_party" "libvpx" "../../webrtc-deps/libvpx/"
preparelink "third_party" "libyuv" "../../webrtc-deps/libyuv/"

echo
echo WebRTC ready.
echo
