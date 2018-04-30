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
call:makepen 255 0 0
:loop
set image=mouse -1
title errorlevel=!errorlevel!
set mouse=!image!
for /f "tokens=1,2,3" %%a in ("!mouse!") do set /a x=%%a,y=%%b
rem set image=cls
cls
set image=draw bin 0 0
set image=draw pen !x! !y!
set image=text "image=!mouse!" 0 600
set image=debug
goto loop

:makepen
set image=target pen
set image=resize pen 1 1
set image=setpix 0 0 %1 %2 %3
set image=resize pen 50 50
set image=target cmd
goto:eof