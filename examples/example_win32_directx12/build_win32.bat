@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@REM Important: to build on 32-bit systems, the DX12 backends needs '#define TextureID unsigned long long', so we pass it here.
@set OUT_DIR=Debug
@set OUT_EXE=example_win32_directx12
@set INCLUDES=/I..\.. /I..\..\backends /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared"
@set SOURCES=main.cpp ..\..\backends\dx12.cpp ..\..\backends\win32.cpp ..\..\imgui*.cpp
@set LIBS=d3d12.lib d3dcompiler.lib dxgi.lib
mkdir Debug
cl /nologo /Zi /MD /utf-8 %INCLUDES% /D TextureID=unsigned long long /D UNICODE /D _UNICODE %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%
