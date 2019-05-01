@echo off

SET OPTS=-FC -GR- -EHa- -nologo -Zi -DIMGUI=0
SET CODE_HOME=%cd%
pushd ..\..\build
cl %OPTS% %CODE_HOME%\win32_renderer.cpp -Ferenderer.exe User32.lib Winmm.lib opengl32.lib Gdi32.lib 
popd
