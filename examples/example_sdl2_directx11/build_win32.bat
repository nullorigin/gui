@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Debug
@set OUT_EXE=example_sdl2_directx11
@set INCLUDES=/I..\.. /I..\..\backends /I%SDL2_DIR%\include /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared" /I "%DXSDK_DIR%Include"
@set SOURCES=main.cpp ..\..\backends\sdl2.cpp ..\..\backends\dx11.cpp ..\..\imgui*.cpp
@set LIBS=/LIBPATH:%SDL2_DIR%\lib\x86 SDL2.lib SDL2main.lib /LIBPATH:"%DXSDK_DIR%/Lib/x86" d3d11.lib d3dcompiler.lib shell32.lib
mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:console
