@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Debug
@set OUT_EXE=MO4-GROUP2
@set INCLUDES=/I.. /I..\backends /I..\examples\libs\glfw\include
@set SOURCES=main.cpp Desktop.cpp Taskbar.cpp ..\backends\imgui_impl_glfw.cpp ..\backends\imgui_impl_opengl3.cpp ..\imgui*.cpp
@set LIBS=/LIBPATH:..\examples\libs\glfw\lib-vc2010-32 glfw3.lib opengl32.lib gdi32.lib shell32.lib
@if not exist %OUT_DIR% mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%