@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=save bin.bmp bin.jpg
set image=save bin.bmp bin.png
pause