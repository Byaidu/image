@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=buffer pen
set image=load bin bin.bmp
set image=load bin2 bin2.bmp
set image=target bin
set image=draw bin2 300 0
set image=target cmd
set image=font 0 255 255 50 100
goto loop
set image=list test.txt
set image=text ÄãºÃ 0 600
set image=buffer pen
:loop
set image=mouse -1
title errorlevel=!errorlevel!
set mouse=!image!
for /f "tokens=1,2,3" %%a in ("!mouse!") do set /a x=%%a,y=%%b
set image=cls
set image=draw bin 0 0
call:makepen 255 0 0
set image=draw pen !x! !y!
set image=text "image=!mouse!" 0 600
rem set image=debug
goto loop

set image=mouse -1 0,0,500,500 0,500,500,1000 500,0,1000,500 500,500,1000,1000
for /f "tokens=1,2,3" %%a in ("!image!") do set /a x=%%a,y=%%b,r=%%a/5%%256,b=%%b/5%%256,g=%%c*100%%256
call:makepen
set image=draw pen !x! !y!
goto loop

:makepen
set image=target pen
set image=resize pen 1 1
set image=setpix 0 0 %1 %2 %3
set image=resize pen 50 50
set image=target cmd
goto:eof