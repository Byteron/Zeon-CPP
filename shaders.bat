@echo off
setlocal
cd /d "%~dp0"
if not exist "bin\shaders" md "bin\shaders"

set "SC=%cd%\deps\prefix\bin\shadercross.exe"
if exist "%SC%" (
  rem Bundled shadercross loads SDL3_shadercross / SPIRV DLLs from prefix\lib
  set "PATH=%cd%\deps\prefix\lib;%cd%\deps\prefix\bin;%PATH%"
  goto have_sc
)

where shadercross >nul 2>&1
if errorlevel 1 (
  echo shadercross not found. Run deps\build_deps.bat or add shadercross to PATH.
  exit /b 1
)
set "SC=shadercross"

:have_sc
"%SC%" assets\shaders\solid_skinned.vert.hlsl -s HLSL -d SPIRV -t vertex -e main -o bin\shaders\solid_skinned.vert.spv
if errorlevel 1 exit /b 1

"%SC%" assets\shaders\solid_skinned.frag.hlsl -s HLSL -d SPIRV -t fragment -e main -o bin\shaders\solid_skinned.frag.spv
if errorlevel 1 exit /b 1

endlocal
