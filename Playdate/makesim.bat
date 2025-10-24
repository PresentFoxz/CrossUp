@echo off
set BUILD_TYPE=Simulator
echo Cleaning and rebuilding CrossUp...

cd /d E:\PlaydateSDK\C_API\CrossUp\Playdate

if exist build (
    echo Removing old build directory...
    rmdir /s /q build
)

echo Creating build directory...
mkdir build
cd build

echo Running CMake...
cmake -G "NMake Makefiles" ..

echo Building with nmake...
nmake

echo Done.
pause