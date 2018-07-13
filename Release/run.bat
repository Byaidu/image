@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin bin.bmp 
:loop
set image=rotate bin 3
set image=draw bin 0 0
echo !image!&!pause!
goto loop