@echo off
if not exist bin mkdir bin
pushd bin

set SOURCES=..\src\ppm.c
cl.exe /Zi /nologo ..\main.c %SOURCES% /I..\include /Feppmviewer.exe /link user32.lib gdi32.lib

popd