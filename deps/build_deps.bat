@echo off
setlocal
cd /d "%~dp0.."
set "ROOT=%CD%"
set "PREFIX=%ROOT%\deps\prefix"
set "BUILD=%ROOT%\deps\build"

if not exist "%ROOT%\deps\SDL\CMakeLists.txt" (
  echo Cloning SDL...
  git clone --depth 1 https://github.com/libsdl-org/SDL.git "%ROOT%\deps\SDL"
  if errorlevel 1 exit /b 1
)
if not exist "%ROOT%\deps\SDL_shadercross\CMakeLists.txt" (
  echo Cloning SDL_shadercross...
  git clone --recursive https://github.com/libsdl-org/SDL_shadercross.git "%ROOT%\deps\SDL_shadercross"
  if errorlevel 1 exit /b 1
)

pushd "%ROOT%\deps\SDL_shadercross"
git submodule update --init --recursive
if errorlevel 1 popd & exit /b 1
popd

if not exist "%BUILD%\SDL" mkdir "%BUILD%\SDL"
cmake -S "%ROOT%\deps\SDL" -B "%BUILD%\SDL" -DCMAKE_INSTALL_PREFIX="%PREFIX%" -DSDL_SHARED=ON -DSDL_STATIC=OFF
if errorlevel 1 exit /b 1
cmake --build "%BUILD%\SDL" --config Release --parallel --target install
if errorlevel 1 exit /b 1

if exist "%BUILD%\SDL_shadercross\CMakeCache.txt" (
  findstr /C:"SDLSHADERCROSS_VENDORED:BOOL=ON" "%BUILD%\SDL_shadercross\CMakeCache.txt" >nul 2>&1
  if errorlevel 1 (
    echo Removing stale SDL_shadercross build ^(reconfigure with VENDORED=ON^)...
    rmdir /s /q "%BUILD%\SDL_shadercross"
  )
)
if not exist "%BUILD%\SDL_shadercross" mkdir "%BUILD%\SDL_shadercross"
cmake -S "%ROOT%\deps\SDL_shadercross" -B "%BUILD%\SDL_shadercross" -DCMAKE_INSTALL_PREFIX="%PREFIX%" -DCMAKE_PREFIX_PATH="%PREFIX%" -DSDLSHADERCROSS_VENDORED=ON -DSDLSHADERCROSS_INSTALL=ON -DCMAKE_INSTALL_RPATH="$ORIGIN/../lib" -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON
if errorlevel 1 exit /b 1
cmake --build "%BUILD%\SDL_shadercross" --config Release --parallel --target install
if errorlevel 1 (
  echo Warning: shadercross install target failed; checking build tree...
)

if not exist "%PREFIX%\bin" mkdir "%PREFIX%\bin"
if not exist "%PREFIX%\bin\shadercross.exe" (
  if exist "%BUILD%\SDL_shadercross\Release\shadercross.exe" copy /Y "%BUILD%\SDL_shadercross\Release\shadercross.exe" "%PREFIX%\bin\shadercross.exe"
)
if not exist "%PREFIX%\bin\shadercross.exe" (
  if exist "%BUILD%\SDL_shadercross\shadercross.exe" copy /Y "%BUILD%\SDL_shadercross\shadercross.exe" "%PREFIX%\bin\shadercross.exe"
)

echo deps ready at %PREFIX%
endlocal
