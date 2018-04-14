@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=buffer pen
goto loop
set image=list test.txt
set image=text ÄãºÃ 0 600
set image=buffer pen
:loop
set image=mouse -1 0,0,500,500 0,500,500,1000 500,0,1000,500 500,500,1000,1000
title errorlevel=!errorlevel!
for /f "tokens=1,2,3" %%a in ("!image!") do set /a x=%%a,y=%%b,r=%%a/5%%256,b=%%b/5%%256,g=%%c*100%%256
call:makepen
set image=draw pen !x! !y!
goto loop

:makepen
set image=target pen
set image=resize pen 1 1
set image=setpix 0 0 !r! !g! !b!
set image=resize pen 50 50
set image=target cmd
goto:eof