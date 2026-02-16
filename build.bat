@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build

echo * > build/.gitignore

:: set filename=001_colored_background.cpp
:: set filename=002_colored_triangle_no_vertices.cpp
:: set filename=003_colored_triangle_with_vertices.cpp
:: set filename=004_texture.cpp
:: set filename=dx11_crash.cpp
:: set filename=006_perspective_projection_matrix.cpp
:: set filename=007_3d.cpp
:: set filename=008_camera.cpp
:: set filename=009_light.cpp
:: set filename=010_point_light.cpp
set filename=geo.cpp

cl /nologo /W4 /WX /Z7 /Fo.\build\ %filename% /link /out:build\main.exe

build\main