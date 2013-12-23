#!/bin/sh
echo "Please enter your android ndk path:"
echo "For example:/home/manishg/Adhikari/ObjectRTC/android-ndk-r8e"
read Input
echo "You entered:$Input"

echo "----------------- Exporting the android-ndk path ----------------"

#Set path
export PATH=$PATH:$Input:$Input/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin

#create install directories
mkdir -p ./../../build
mkdir -p ./../../../build/android

#webrtc module build
echo "------------------- Building webrtc for ANDROID platform ---------------"
pushd `pwd`
mkdir -p ./../../../build/android/webrtc

export ANDROID_NDK_PATH=$Input
echo "------------------- Applying patches for webrtc make file------------"
cd ./../../
rm -rf ./out

make All BUILDTYPE=Release
popd

echo "-------- Installing webrtc libs -----"
cp -r ./../../out/Release/lib* ./../../../build/android/webrtc/

#clean
rm -rf ./../../out

