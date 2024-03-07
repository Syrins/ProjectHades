@echo off
set today = %date%

echo save date : %date%
echo %today%

sc create TECH binPath="%~dp0driver.sys" type=Kernel
date 22-11-2017
sc start TECH
date %today%
pause