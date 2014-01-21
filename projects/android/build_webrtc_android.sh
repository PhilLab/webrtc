#!/bin/sh

#Note [TBD] : There is no check for ndk-version
#Please use the ndk-version as per host machine for now

#Get the machine type 
PROCTYPE=`uname -m`
if [ "$PROCTYPE" = "i686" ] || [ "$PROCTYPE" = "i386" ] || [ "$PROCTYPE" = "i586" ] ; then
        echo "Host machine : x86"
        ARCHTYPE="x86"
else
        echo "Host machine : x86_64"
        ARCHTYPE="x86_64"
fi

#NDK-path
if [[ $1 == *ndk* ]]; then
	echo "----------------- NDK Path is : $1 ----------------"
	Input=$1;
else
	echo "Please enter your android ndk path:"
	echo "For example:/home/astro/android-ndk-r8e"
	read Input
	echo "You entered:$Input"
fi

#depot-tools set
if [ -d depot_tools ] ; then
	export PATH=$PATH:`pwd`/depot_tools
else
	tar xvf depot_tools.tar.gz
	export PATH=$PATH:`pwd`/depot_tools
fi

#Set path
echo "----------------- Exporting the android-ndk path ----------------"
if [ "$ARCHTYPE" = "x86" ] ; then
	export PATH=$PATH:$Input:$Input/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin
else
        export PATH=$PATH:$Input:$Input/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86_64/bin
fi

#create install directories
mkdir -p ./../../build
mkdir -p ./../../../build/android

#webrtc module build
echo "------------------- Building webrtc for ANDROID platform ---------------"
pushd `pwd`
mkdir -p ./../../../build/android/webrtc

export ANDROID_NDK_PATH=$Input
cd ./../../
rm -rf ./out

#Generaing the make files
export GYP_GENERATORS=make

GYP_DEFINES="host_os=linux android_host_arch=$ARCHTYPE OS=android target_arch=arm android_ndk_root=$Input" gclient runhooks

make BUILDTYPE=Release ARFLAGS.target=crs

cd ./out

echo "-------- Installing webrtc libs -----"
#cp -r ./../../out/Release/obj.target/*.a ./../../../build/android/webrtc/
find -type f -iname '*.a' -exec cp {} ./../../build/android/webrtc/ \;

popd

#clean
rm -rf ./../../out

