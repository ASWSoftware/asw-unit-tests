@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "TARGET_DIR=%~1"

if "%TARGET_DIR%"=="" (
    echo ERROR: A staging/deploy folder is required.
    echo Usage: Deploy.bat "C:\path\to\staging"
    exit /b 1
)

echo Deploying ASWUnitTests files to:
echo   %TARGET_DIR%

if not exist "%TARGET_DIR%\." (
    echo Creating target folder...
    mkdir "%TARGET_DIR%"
    if errorlevel 1 (
        echo ERROR: Could not create target folder.
        exit /b 1
    )
)

for %%D in (build cmake rad370 src tests toTest) do (
    if not exist "%TARGET_DIR%\%%D\." (
        echo Creating %%D folder...
        mkdir "%TARGET_DIR%\%%D"
        if errorlevel 1 (
            echo ERROR: Could not create "%TARGET_DIR%\%%D".
            exit /b 1
        )
    )
)

call :CopyFile "LICENSE" "LICENSE"
if errorlevel 1 exit /b 1
call :CopyFile "README.md" "README.md"
if errorlevel 1 exit /b 1
call :CopyFile "cmake\CMakeLists.txt" "cmake\CMakeLists.txt"
if errorlevel 1 exit /b 1
call :CopyFile "rad370\ASWUnitTests.cbproj" "rad370\ASWUnitTests.cbproj"
if errorlevel 1 exit /b 1
call :CopyFile "rad370\ASWUnitTestsPCH1.h" "rad370\ASWUnitTestsPCH1.h"
if errorlevel 1 exit /b 1
call :CopyFile "rad370\Build_Win64x_Debug.bat" "rad370\Build_Win64x_Debug.bat"
if errorlevel 1 exit /b 1
call :CopyFile "rad370\Build_Win64x_Release.bat" "rad370\Build_Win64x_Release.bat"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_CLI.cpp" "src\ASWUnitTests_CLI.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_CLI.h" "src\ASWUnitTests_CLI.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Console.cpp" "src\ASWUnitTests_Console.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Console.h" "src\ASWUnitTests_Console.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Exception.cpp" "src\ASWUnitTests_Exception.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Exception.h" "src\ASWUnitTests_Exception.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Handler.cpp" "src\ASWUnitTests_Handler.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Handler.h" "src\ASWUnitTests_Handler.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_JUnitReport.cpp" "src\ASWUnitTests_JUnitReport.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_JUnitReport.h" "src\ASWUnitTests_JUnitReport.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Registry.cpp" "src\ASWUnitTests_Registry.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Registry.h" "src\ASWUnitTests_Registry.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Sources.cmake" "src\ASWUnitTests_Sources.cmake"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_StdOutRedirect.cpp" "src\ASWUnitTests_StdOutRedirect.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_StdOutRedirect.h" "src\ASWUnitTests_StdOutRedirect.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_TestBase.cpp" "src\ASWUnitTests_TestBase.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_TestBase.h" "src\ASWUnitTests_TestBase.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Version.cpp" "src\ASWUnitTests_Version.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "src\ASWUnitTests_Version.h" "src\ASWUnitTests_Version.h"
if errorlevel 1 exit /b 1
call :CopyFile "src\main.cpp" "src\main.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWTools_Random.cpp" "tests\Test_ASWTools_Random.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWTools_Random.h" "tests\Test_ASWTools_Random.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWTools_String.cpp" "tests\Test_ASWTools_String.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWTools_String.h" "tests\Test_ASWTools_String.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_CLI.cpp" "tests\Test_ASWUnitTests_CLI.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_CLI.h" "tests\Test_ASWUnitTests_CLI.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_Console.cpp" "tests\Test_ASWUnitTests_Console.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_Console.h" "tests\Test_ASWUnitTests_Console.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_Handler.cpp" "tests\Test_ASWUnitTests_Handler.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_Handler.h" "tests\Test_ASWUnitTests_Handler.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_JUnitReport.cpp" "tests\Test_ASWUnitTests_JUnitReport.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_JUnitReport.h" "tests\Test_ASWUnitTests_JUnitReport.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_StdOutRedirect.cpp" "tests\Test_ASWUnitTests_StdOutRedirect.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_StdOutRedirect.h" "tests\Test_ASWUnitTests_StdOutRedirect.h"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_TestBase.cpp" "tests\Test_ASWUnitTests_TestBase.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "tests\Test_ASWUnitTests_TestBase.h" "tests\Test_ASWUnitTests_TestBase.h"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_Common.cpp" "toTest\ASWTools_Common.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_Common.h" "toTest\ASWTools_Common.h"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_Random.cpp" "toTest\ASWTools_Random.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_Random.h" "toTest\ASWTools_Random.h"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_String.cpp" "toTest\ASWTools_String.cpp"
if errorlevel 1 exit /b 1
call :CopyFile "toTest\ASWTools_String.h" "toTest\ASWTools_String.h"
if errorlevel 1 exit /b 1

echo Deployment completed successfully.
exit /b 0

:CopyFile
set "SOURCE_FILE=%~1"
set "DESTINATION_FILE=%~2"
echo Copying %SOURCE_FILE%...
if not exist "%SCRIPT_DIR%%SOURCE_FILE%" (
    echo ERROR: Source file not found: "%SCRIPT_DIR%%SOURCE_FILE%"
    exit /b 1
)
copy /Y "%SCRIPT_DIR%%SOURCE_FILE%" "%TARGET_DIR%\%DESTINATION_FILE%" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy "%SOURCE_FILE%".
    exit /b 1
)
exit /b 0