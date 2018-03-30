@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=show cmd
set image=load bin.bmp bin
goto loop

set image=load black.bmp black
set image=buffer buf
for /l %%a in (0,50,1000) do (
  rem cls
  set image=target buf
  set image=draw black 0 0
  set image=draw bin %%a %%a
  set image=target cmd
  set image=draw buf 0 0
  set image=show cmd
  pause
)
set image=draw black 0 0
pause
set image=show cmd
pause
set start=!time:~6,2!

:loop
set /a fps+=2
::set image=draw bin 0 0
::set image=draw bin 800 0
::set image=draw bin !fps! 0
if not "!time:~6,2!"=="!start!" set start=!time:~6,2!&title !fps!&set fps=0
goto loop