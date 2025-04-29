@echo off
if not exist "bin"  md "bin"
REM /Bt+ for timing
set CompilerFlags=/std:c++20 /EHsc /Zi /Yupch.h /Fpbin\pch.pch /nologo

set IncludeFlags=/Ivendor\cgltf /Ivendor\stb

set SourceFlags=src\main.cpp

set LibraryFlags=bin\pch.obj vendor\SDL3\lib\x64\SDL3.lib

cl %CompilerFlags% %IncludeFlags% %SourceFlags% %LibraryFlags% /Fobin\ /Febin\main.exe /Fdbin\ /link /nologo

copy vendor\SDL3\lib\x64\SDL3.dll bin\SDL3.dll