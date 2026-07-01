@echo off
setlocal

set WINAVR_DST=D:\users\bernd\PRIVAT\winavr\pvsim
set EAGLE_DST=D:\users\bernd\PRIVAT\eagle\pvsim
set GIT_DIR=D:\users\bernd\PRIVAT\github\pvsim

set WINAVR_SRC=%GIT_DIR%\WinAVR
set EAGLE_SRC=%GIT_DIR%\Eagle

echo ACHTUNG: Arbeitsdateien werden aus der GitHub-Kopie ueberschrieben.
set /p OK=Fortfahren? J/N: 

if /I not "%OK%"=="J" (
    echo Abgebrochen.
    pause
    exit /b 0
)

cd /d "%GIT_DIR%"
git pull --rebase origin main
if errorlevel 1 (
    echo Fehler beim Pull.
    pause
    exit /b 1
)

echo Stelle WinAVR wieder her...
robocopy "%WINAVR_SRC%" "%WINAVR_DST%" /MIR /XD default alt .git /XF id_rsa id_rsa.pub
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

echo Wiederherstellung abgeschlossen.
pause