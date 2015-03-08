@echo off
echo.
echo Preparing symbolic links for WebRTC...
echo.

set failure=0

call:dolink build ..\webrtc-deps\webrtc-build
if "%failure%" neq "0" goto:done_with_error

goto:done


:dolink
IF EXIST .\%~1\nul goto:eof
IF NOT EXIST %~2\nul call:failure -1 %~1 %~2 "%~2 does not exist!"
if "%failure%" neq "0" goto:eof

echo creating symbolic link for "%~1" to "%~2"
mklink /J %~1 %~2
if %errorlevel% neq 0 call:failure %errorlevel% %~1 %~2 "Could not create symbolic link to %~1 from %~2"
if "%failure%" neq "0" goto:eof

goto:eof

:failure
echo.
echo ERROR: %~4
echo.

set failure=%~1

goto:eof

:done_with_error

exit /b %failure%
goto:eof

:done

echo.
echo Success: WebRTC ready.
echo.
