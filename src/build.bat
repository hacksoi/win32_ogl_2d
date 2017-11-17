@echo off

pushd ..\build
cl /nologo /Od /Zi /W4 /wd4100 /wd4201 /wd4505 /wd4189 /wd4101 /wd4005 /wd4996 /I..\..\utils ..\src\win32_ogl_2d.cpp User32.lib Kernel32.lib Gdi32.lib OpenGL32.lib
popd
