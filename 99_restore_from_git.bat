@echo off
setlocal

set WINAVR_DST=D:\users\bernd\PRIVAT\winavr\pvsim
set EAGLE_DST=D:\users\bernd\PRIVAT\eagle\pvsim
set LOC_DIR=D:\users\bernd\PRIVAT\github\pvsim

set WINAVR_SRC=%LOC_DIR%\WinAVR
set EAGLE_SRC=%LOC_DIR%\Eagle

echo ACHTUNG: Arbeitsdateien werden aus der GitHub-Kopie ueberschrieben.
set /p OK=Fortfahren? J/N: 

if /I not "%OK%"=="J" (
    echo Abgebrochen.
    pause
    exit /b 0
)

cd /d "%LOC_DIR%"
git pull --rebase origin main
if errorlevel 1 (
    echo Fehler beim Pull.
    pause
    exit /b 1
)

echo Stelle WinAVR wieder her...
robocopy "%WINAVR_SRC%" "%WINAVR_DST%" /MIR /XD default alt .git 
if errorlevel 8 (
    echo Fehler beim Wiederherstellen von WinAVR.
    pause
    exit /b 1
)

echo Stelle Eagle-Dateien wieder her...
robocopy "%EAGLE_SRC%" "%EAGLE_DST%" *.sch *.brd *.lbr *.cam *.dru *.ulp
if errorlevel 8 (
    echo Fehler beim Wiederherstellen von Eagle.
    pause
    exit /b 1
)
echo \Fertigung und \Doku muesste hier noch folgen
echo Wiederherstellung abgeschlossen.
pause