@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
set image=sleep 1000
set image=save desktop srceen.bmp
pause