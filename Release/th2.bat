@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
:loop
set image=getvar msg
title msg:!msg!
set /a color=(color+1)%%256
set image=buffer buf 100 100 !color! 0 0
set image=draw buf 100 0
set image=sleep 5
goto loop