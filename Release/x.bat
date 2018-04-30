@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=draw button1.bmp 500 500
set image=list x.txt init

:act1
set image=list x.txt act1
set image=list x.txt !errorlevel!
if "!errorlevel!"=="cmd" goto act1
call:act1_!errorlevel!
goto act2

:act1_button1
rem to do something
goto:eof

:act1_button2
rem to do something
goto:eof

:act2
set image=list x.txt act2
set image=list x.txt !errorlevel!
if "!errorlevel!"=="cmd" goto act2

:act3
set image=list x.txt act3
set image=list x.txt !errorlevel!
if "!errorlevel!"=="cmd" goto act3

echo ÑÝÊ¾Íê±Ï
pause