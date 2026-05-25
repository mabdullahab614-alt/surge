@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set CMAKE="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set NINJA_DIR="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
set PATH=%NINJA_DIR%;%PATH%

echo === Configuring ===
%CMAKE% -B "d:\my ML projects\game\build" -S "d:\my ML projects\game" -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error

echo === Building ===
%CMAKE% --build "d:\my ML projects\game\build" --config Release
if errorlevel 1 goto :error

echo === Done! Run: d:\my ML projects\game\build\Platformer.exe ===
goto :end

:error
echo BUILD FAILED
:end
