@echo off

SET LIB_INCLUDE_PATHS=/I..\libraries\glew\include /I..\libraries\stb\include /I..\libraries\nps_utils\include
SET LIB_PATHS=/LIBPATH:..\libraries\glew\bin

SET COMMON_COMPILER_FLAGS=/nologo /Od /Zi /W4 /wd4100 /wd4201 /wd4505 /wd4189 /wd4101 /wd4005 /wd4996
set COMMON_LINKER_FLAGS=/opt:ref /incremental:no

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build
cl %COMMON_COMPILER_FLAGS% %LIB_INCLUDE_PATHS% ..\src\platform_win32.cpp /link %COMMON_LINKER_FLAGS% %LIBPATHS% User32.lib Kernel32.lib Gdi32.lib OpenGL32.lib
popd
