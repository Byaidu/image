@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
pause
set image=buffer box 600 480 0 0 255
set image=load bin bin.bmp
set image=[cmd] draw bin 0 0
set image=[box] draw bin 0 0
set image=show box
set image=target box
set image=draw bin 100 100
rem set image=hide cmd
:loop
set image=sleep 10
set image=getmsg box
for %%a in (!box.wm!) do for /f "tokens=1-8 delims=." %%b in ("%%a") do (
  if "%%b"=="WM_MOUSEMOVE" set image=line 0 0 %%c %%d
  if "%%b"=="WM_DESTROY" exit
)
goto loop