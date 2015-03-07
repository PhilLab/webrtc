@echo off
echo PREPARING SYMBOLIC LINKS WITHIN WEBRTC...
echo.

IF EXIST .\build\nul goto :exists_build
mklink /J build "..\webrtc-build"

:exists_build

echo.
echo DONE.
