@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin.bmp bin
set image=draw bin 500 500
:loop
set image=rotate cmd 1
rem set image=draw bin 0 0
goto loop