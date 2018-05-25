@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=pen -1
set image=brush 0 0 255
set image=ellipse 0 0 1000 500
set image=polygon 0,0 500,0 1000,1000 0,500
set image=line 0 1000 1000 0
pause