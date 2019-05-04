@echo off

SET OPTS=-FC -GR- -EHa- -nologo -Zi -DIMGUI=0 -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 /Od /fp:fast /fp:except- -Zo -Oi -GS- -Gs9999999
set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref User32.lib Winmm.lib opengl32.lib Gdi32.lib 
SET CODE_HOME=%cd%
pushd ..\..\build
cl %OPTS% %CODE_HOME%\win32_renderer.cpp -Ferenderer.exe /link /SUBSYSTEM:windows %CommonLinkerFlags% 
popd
