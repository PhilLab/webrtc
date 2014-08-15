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

#Get the Host OS
HOST_OS=`uname -s`
case "$HOST_OS" in
    Darwin)
        HOST_OS=darwin
        ;;
    Linux)
        HOST_OS=linux
        ;;
esac

#GNU Toolchain Version
TOOLCHAIN_VERSION=4.7

#NDK-path
if [[ $1 == *ndk* ]]; then
	echo "----------------- NDK Path is : $1 ----------------"
	Input1=$1;
else
	echo "Please enter your android ndk path:"
	echo "For example:/home/astro/android-ndk-r8e"
	read Input1
	echo "You entered:$Input1"
fi

#NDK-path
if [[ $2 == *sdk* ]]; then
        echo "----------------- SDK Path is : $2 ----------------"
        Input2=$2;
else
        echo "Please enter your android sdk path:"
        echo "For example:/home/astro/android-sdk"
        read Input2
        echo "You entered:$Input2"
fi

#create install directories
mkdir -p ./../../build
mkdir -p ./../../../build/android

#webrtc module build
echo "------------------- Building webrtc for ANDROID platform ---------------"
pushd `pwd`
mkdir -p ./../../../build/android/webrtc

export ar=$Input1/toolchains/arm-linux-androideabi-$TOOLCHAIN_VERSION/prebuilt/$HOST_OS-$ARCHTYPE/bin/arm-linux-androideabi-ar
cd ./../../

#Avoid generation of make files on mac platform
if [ "$HOST_OS" == "linux" ] ; then
	echo "------------------- Linux platform is not supported ---------------"
	exit 1
	#Generaing the make files
#	export GYP_GENERATORS=make
#	GYP_DEFINES="host_os=linux android_host_arch=$ARCHTYPE OS=android target_arch=arm android_ndk_root=$Input1" gclient runhooks
#	make BUILDTYPE=Release PLATFORM=$HOST_OS-$ARCHTYPE ARFLAGS.target=crs
elif [ "$HOST_OS" == "darwin" ] ; then
	echo "android_ndk_path = $Input1" > ./projects/android/ninja/conf.ninja
	echo "android_sdk_path = $Input2" >> ./projects/android/ninja/conf.ninja
	echo "host_os = $HOST_OS" >> ./projects/android/ninja/conf.ninja
	echo "archtype = $ARCHTYPE" >> ./projects/android/ninja/conf.ninja
	echo "toolchain_version = $TOOLCHAIN_VERSION" >> ./projects/android/ninja/conf.ninja
	
	ninja -C ./projects/android/ninja -v
else
	echo "------------------- Unsupported Platform ---------------"
	exit 1
fi

echo "-------- Installing webrtc libs -----"
find projects/android/ninja -iname '*.a' -or -iname '*.jar' -exec cp {} ./../build/android/webrtc/ \;

popd

#clean
#ninja -C ./ninja -t clean
