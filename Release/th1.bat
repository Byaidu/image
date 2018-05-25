@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
:loop
set /a color=(color+1)%%256
set image=buffer buf 100 100 0 0 !color!
set image=draw buf 0 0
set image=sleep 10
goto loop