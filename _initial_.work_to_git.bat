@echo off
setlocal

set WINAVR_SRC=D:\users\bernd\PRIVAT\winavr\pvsim
set EAGLE_SRC=D:\users\bernd\PRIVAT\eagle\pvsim
set GIT_DIR=D:\users\bernd\PRIVAT\github\pvsim

set WINAVR_DST=%GIT_DIR%\WinAVR
set EAGLE_DST=%GIT_DIR%\Eagle

echo Kopiere WinAVR...
robocopy "%WINAVR_SRC%" "%WINAVR_DST%" /MIR /XD default alt .git /XF id_rsa id_rsa.pub
if errorlevel 8 (
    echo Fehler beim Kopieren von WinAVR.
    pause
    exit /b 1
)

echo Kopiere Eagle gefiltert...
del /Q "%EAGLE_DST%\*.*" 2>nul
for /D %%d in ("%EAGLE_DST%\*") do rd /S /Q "%%d"

robocopy "%EAGLE_SRC%" "%EAGLE_DST%" *.sch *.brd *.lbr *.cam *.dru *.ulp 
robocopy "%EAGLE_SRC%\Fertigung" "%EAGLE_DST%\Fertigung" /MIR
robocopy "%EAGLE_SRC%\Doku" "%EAGLE_DST%\Doku" /MIR
if errorlevel 8 (
    echo Fehler beim Kopieren von Eagle.
    pause
    exit /b 1
)


pause