@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build

cl /nologo /W4 /WX /Z7 /fsanitize=address /Fo.\build\ main.cpp /link /out:build\main.exe

build\main