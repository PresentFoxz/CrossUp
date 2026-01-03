@echo off
echo Cleaning and rebuilding CrossUp...

cd /d C:\Users\vcapr\Documents\PlaydateSDK\C_API\CrossUp\Playdate

if exist build (
    echo Removing old build directory...
    rmdir /s /q build
)

echo Creating build directory...
mkdir build
cd build

echo Running CMake...
cmake -G "NMake Makefiles" ^
 --toolchain=C:/Users/vcapr/Documents/PlaydateSDK/C_API/buildsupport/arm.cmake ^
 -DCMAKE_BUILD_TYPE=Release ^
 ..

if errorlevel 1 (
    echo CMake failed. Exiting...
    pause
    exit /b
)

echo Building with nmake...
nmake

echo Done.
pause