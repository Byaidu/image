@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=list test.txt
set image=text ÄãºÃ 0 600
set image=buffer pen
set image=resize pen 1 1
set image=target pen
set image=setpix 0 0 255 0 0
set image=resize pen 20 20
set image=target cmd
rem pause
:loop
set image=mouse
title %image%
set image=draw pen %image%
goto loop