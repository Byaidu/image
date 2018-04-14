@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=show cmd
set image=load bin.bmp bin
:loop
set /a fps+=2
set image=draw bin 0 0
set image=draw bin !fps! 0
if not "!time:~6,2!"=="!start!" set start=!time:~6,2!&title fps=!fps!&set fps=0
goto loop