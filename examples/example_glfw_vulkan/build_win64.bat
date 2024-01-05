@REM Build for Visual Studio compiler. Run your copy of amd64/vcvars32.bat to setup 64-bit command-line compiler.

@set INCLUDES=/I..\.. /I..\..\backends /I..\libs\glfw\include /I %VULKAN_SDK%\include
@set SOURCES=main.cpp ..\..\backends\vulkan.cpp ..\..\backends\glfw.cpp ..\..\imgui*.cpp
@set LIBS=/LIBPATH:..\libs\glfw\lib-vc2010-64 /libpath:%VULKAN_SDK%\lib glfw3.lib opengl32.lib gdi32.lib shell32.lib vulkan-1.lib

@set OUT_DIR=Debug
mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 %INCLUDES% /D TextureID=U64 %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%

@set OUT_DIR=Release
mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 /Ox /Oi %INCLUDES% /D TextureID=U64 %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%
