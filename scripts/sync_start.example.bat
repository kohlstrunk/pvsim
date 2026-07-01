@echo off
setlocal

set LOC_DIR=D:\PFAD\ZUM\REPO

cd /d "%LOC_DIR%"

echo Bitte die Pfade anpassen
pause

git pull --rebase origin main
if errorlevel 1 (
    echo Fehler beim Pull.
    pause
    exit /b 1
)

echo GitHub-Kopie ist aktuell.
pause