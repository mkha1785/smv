@echo off
setlocal
call ..\scripts\setopts %*
title Building zlib library
erase *.o *.obj libz.a libz.lib
set target=libz.lib
if %COMPILER% == gcc set target=libz.a
make COMPILER=%COMPILER% SIZE=%SIZE% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
