@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin bin.bmp
:loop
set image=rotate bin 30
set image=draw bin 0 0
set image=sleep 50
goto loop