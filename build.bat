@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build

@REM Build options
set obj_output=/Fo".\\build\\"
set disable_annoying_warnings=/wd4189 /wd4100 /wd4700

set build_options=/nologo /W4 /WX /O2 %obj_output% %disable_annoying_warnings%

cl %build_options% main.cpp /link /incremental:no /subsystem:windows /out:build\main.exe

build\main