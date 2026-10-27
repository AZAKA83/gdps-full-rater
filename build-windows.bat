@echo off
setlocal
if "%GEODE_SDK%"=="" (
  echo ERROR: Set GEODE_SDK to your installed Geode SDK folder first.
  echo Example: set GEODE_SDK=C:\path\to\geode\sdk
  exit /b 1
)
where geode >nul 2>nul
if errorlevel 1 (
  echo ERROR: Geode CLI was not found in PATH. Install it using the official Geode developer docs.
  exit /b 1
)
geode build
if errorlevel 1 exit /b %errorlevel%
echo Build finished. Verify the generated .geode file before installing.
