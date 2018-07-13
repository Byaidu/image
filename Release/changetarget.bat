@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
pause
set image=buffer box 600 480 0 0 255
set image=load bin bin.bmp
set image=[cmd] draw bin 0 0
set image=[box] draw bin 0 0
set image=show box
set image=[box] draw bin 100 100
rem set image=hide cmd
set box.wm= 
:loop
if not "!box.wm!"==" " for %%a in (!box.wm!) do (
	for /f "tokens=1 delims=." %%b in ("%%a") do if [%%b]==[WM_DESTROY] (exit)
	ECHO %%a
)
set box.wm= 
goto loop
pause