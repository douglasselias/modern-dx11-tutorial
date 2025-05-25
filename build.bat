@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build

@REM set filename=001_colored_background.cpp
@REM set filename=002_single_color_triangle.cpp
set filename=003_multi_colored_triangle.cpp

cl /nologo /W4 /WX /Z7 /fsanitize=address /Fo.\build\ %filename% /link /out:build\main.exe

build\main