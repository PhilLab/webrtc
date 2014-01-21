webrtc
======

Dependency library webrtc for WebRTC engine (as used in Open Peer C++ library)

WEBRTC ANDROID BUILD :
------------------------
Steps to build android webrtc libraries:
<pre>
<code>
cd ./projects/android
./build_webrtc_android.sh <NDK ROOT PATH>
</code>
</pre>

Notes :
- NDK absolute path for "android-ndk-r8e" is required for build.
- After successful build, submodule built libs can be found at : ./../libs/build/android/webrtc directory.
- ./../.gclient file required by gyp program for make file generation.
- Tested with NDK-r8e for x86 and x86_64

known Issues in android build :
- Linking webrtc libraries while building ortclib test shared libary.

For more information on ORTC, please visit:
http://ortc.org/

