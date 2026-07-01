@echo off
setlocal

set LOC_DIR=D:\PFAD\ZUM\REPO
set WINAVR_SRC=D:\PFAD\ZU\WINAVR
set EAGLE_SRC=D:\PFAD\ZU\EAGLE
set WINAVR_DST=%LOC_DIR%\WinAVR
set  EAGLE_DST=%LOC_DIR%\Eagle

echo Bitte die Pfade anpassen
pause

echo Kopiere WinAVR...

robocopy "%WINAVR_SRC%" "%WINAVR_DST%" /MIR /XD default alt .git 
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

cd /d "%LOC_DIR%"

git add -A

git diff --cached --quiet
if %errorlevel%==0 (
    echo Keine Aenderungen zum Sichern.
    pause
    exit /b 0
)

set /p MSG=Commit-Nachricht eingeben: 

if "%MSG%"=="" (
    set MSG=Zwischenspeicherung %date% %time%
)

git commit -m "%MSG%"
if errorlevel 1 (
    echo Fehler beim Commit.
    pause
    exit /b 1
)

git pull --rebase origin main
if errorlevel 1 (
    echo Fehler beim Pull. Konflikte pruefen.
    pause
    exit /b 1
)

git push origin main
if errorlevel 1 (
    echo Fehler beim Push.
    pause
    exit /b 1
)

echo Upload erfolgreich.
pause