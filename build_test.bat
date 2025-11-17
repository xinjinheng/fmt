@echo off
setlocal

REM 设置编译器和编译选项
g++ -std=c++11 test_complete_features.cpp src/format.cc src/cross_format.cc src/distributed_format.cc -Iinclude -o test_complete_features.exe

REM 检查编译结果
if %errorlevel% equ 0 (
    echo Compilation successful!
    echo Running test...
    test_complete_features.exe
) else (
    echo Compilation failed!
)

endlocal