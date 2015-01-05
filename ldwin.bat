@ECHO OFF
arm-none-eabi-nm %~5 | gawk '/__wrap_/ {print $3} | sed 's/__wrap_/-Wl,--wrap=/g' | tr '\n' ' ' > lwrap
set /p LWRAP=<lwrap
@ECHO ON
%~1 %~2 %LWRAP% %~3 %~4 %~5