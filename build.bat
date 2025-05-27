@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build

@REM set filename=001_colored_background.cpp
@REM set filename=002_single_color_triangle.cpp
@REM set filename=003_multi_colored_triangle.cpp
@REM set filename=004_texture.cpp
@REM set filename=005_indices.cpp
@REM set filename=006_perspective_projection_matrix.cpp
set filename=007_3d.cpp
@REM set filename=png_loader.cpp

cl /nologo /W4 /WX /Z7 /fsanitize=address /Fo.\build\ %filename% /link /out:build\main.exe

build\main