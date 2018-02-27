@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin.bmp bin
:loop
set image=rotate bin 1
set image=draw bin 0 0
goto loop