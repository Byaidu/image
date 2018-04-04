@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=list test.txt
set image=text ÄãºÃ 0 600
pause