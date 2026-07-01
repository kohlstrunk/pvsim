@echo off
setlocal
set LOC_DIR=D:\users\bernd\PRIVAT\github\pvsim
set  EAGLE_DST=%LOC_DIR%\Eagle
set EAGLE_SRC=D:\users\bernd\PRIVAT\eagle\pvsim
robocopy "%EAGLE_SRC%\Fertigung" "%EAGLE_DST%\Fertigung" /MIR
robocopy "%EAGLE_SRC%\Doku" "%EAGLE_DST%\Doku" /MIR

git status
pause