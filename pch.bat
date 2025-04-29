@echo off
if not exist "bin"  md "bin"

set CompilerFlags=/std:c++20 /EHsc /Zi /nologo

set IncludeFlags=/Ivendor\SDL3\include /Ivendor\handmade_math

cl %CompilerFlags% %IncludeFlags% /Ycpch.h /Fpbin\pch.pch /c src\pch.cpp /Fobin\ /Fdbin\