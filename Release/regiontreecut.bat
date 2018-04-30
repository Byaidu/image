@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=resize cmd 1000 1000
set image=buffer p1 1000 500 255 0 0
set image=buffer p2 500 1000 0 0 255
set image=buffer frame1
set image=buffer frame2
set image=target frame1
set image=draw p1 0 0
set image=draw p1 0 500
set image=target frame2
set image=draw p2 0 0
set image=draw p2 500 0
set image=target cmd
set image=draw frame1 0 0
pause
set image=debug
pause
set image=draw frame2 0 0
pause
set image=debug
pause
set image=union cmd
set image=debug
pause