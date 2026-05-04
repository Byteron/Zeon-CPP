@echo off
setlocal
cd /d "%~dp0"

if defined SDL_PREFIX (
  set "SDL_INCLUDE=%SDL_PREFIX%\include"
  set "SDL_LIBPATH=%SDL_PREFIX%\lib"
  set "SDL_DLL_SRC=%SDL_PREFIX%\bin\SDL3.dll"
) else (
  if not exist "deps\prefix\lib\SDL3.lib" (
    echo SDL3 not in deps\prefix; running deps\build_deps.bat ...
    call deps\build_deps.bat
    if errorlevel 1 exit /b 1
  )
  set "SDL_INCLUDE=%cd%\deps\prefix\include"
  set "SDL_LIBPATH=%cd%\deps\prefix\lib"
  set "SDL_DLL_SRC=%cd%\deps\prefix\bin\SDL3.dll"
)

if not exist "bin" md "bin"

call shaders.bat
if errorlevel 1 exit /b 1

set CompilerFlags=/std:c++20 /EHsc /Zi /nologo
set IncludeFlags=/Ivendor\cgltf /Ivendor\stb /I"%SDL_INCLUDE%"

cl %CompilerFlags% %IncludeFlags% src\main.cpp /Fobin\ /Febin\main.exe /Fdbin\ /link /nologo /LIBPATH:"%SDL_LIBPATH%" SDL3.lib
if errorlevel 1 exit /b 1

if exist "%SDL_DLL_SRC%" copy /Y "%SDL_DLL_SRC%" bin\SDL3.dll >nul

echo Built %cd%\bin\main.exe
endlocal
