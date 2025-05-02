@echo off
if not exist "bin" md "bin"

set CompilerFlags=/std:c++20 /EHsc /Zi /nologo

set IncludeFlags=/Ivendor\cgltf /Ivendor\stb /Ivendor\SDL3\include

set LibraryFlags=vendor\SDL3\lib\x64\SDL3.lib

cl %CompilerFlags% %IncludeFlags% src\main.cpp %LibraryFlags% /Fobin\ /Febin\main.exe /Fdbin\ /link /nologo

copy vendor\SDL3\lib\x64\SDL3.dll bin\SDL3.dll