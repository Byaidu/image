@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
set image=list %0 script
pause
exit

:script
sleep 10000
save desktop screen.bmp
target desktop
target backup
draw desktop 0 0
target desktop
buffer black 3000 3000 0 0 0
draw black 0 0
sleep 300
draw backup 0 0
exit