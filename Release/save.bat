@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=save bin.jpg bin.bmp
set image=save bin.png bin.bmp
pause