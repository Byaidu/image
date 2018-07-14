@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=full cmd
cmd