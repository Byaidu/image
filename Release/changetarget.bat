@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
pause
set image=buffer box 600 480 0 0 255
set image=load bin bin.bmp
set image=[cmd] draw bin 0 0
set image=[desktop] draw bin 0 0
set image=[box] draw bin 0 0
set image=show box
set image=[box] draw bin 100 100
rem set image=hide cmd
set box.wm= 
:loop
if not "!box.wm!"==" " echo.!box.wm!
set box.wm= 
set image=sleep 100
goto loop
pause