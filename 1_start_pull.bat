@echo off
setlocal

cd /d "D:\users\bernd\PRIVAT\github\pvsim"

git pull --rebase origin main
if errorlevel 1 (
    echo Fehler beim Pull.
    pause
    exit /b 1
)

echo GitHub-Kopie ist aktuell.
pause