@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Debug
@set OUT_EXE=example_win32_opengl3
@set INCLUDES=/I..\.. /I..\..\backends
@set SOURCES=main.cpp ..\..\backends\opengl3.cpp ..\..\backends\win32.cpp ..\..\imgui*.cpp
@set LIBS=opengl32.lib
mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 %INCLUDES% /D UNICODE /D _UNICODE %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%
