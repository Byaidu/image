@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin.jpg bin
set image=rotate bin 45
set image=save bin2.bmp bin
set image=load bin2.bmp bin
set image=draw bin2.bmp 0 0
pause