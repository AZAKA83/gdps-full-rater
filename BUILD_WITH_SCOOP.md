# Build GDPS Full Rater using Scoop on Windows

> This package is a source starter, not a prebuilt `.geode` file. The rating UI does not yet send ratings to a GDPS. Real custom ratings need a backend adapter made for your specific server core.

## 1. Install Scoop

Open a regular (non-Administrator) PowerShell window and run:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
```

Official source: https://scoop.sh/

## 2. Install Geode CLI, CMake, Git, LLVM and Ninja

```powershell
scoop bucket add extras
scoop install geode-sdk-cli
scoop install cmake git ninja llvm
```

Official Geode CLI instructions: https://docs.geode-sdk.org/getting-started/geode-cli/

## 3. Install the Windows C++ build requirements

Install Visual Studio 2022 Build Tools from https://visualstudio.microsoft.com/downloads/. Choose **Desktop development with C++**, including MSVC C++ build tools and the Windows SDK. LLVM alone does not include the Windows SDK and CRT libraries.

## 4. Install SDK and build

Close and reopen PowerShell. Change directory into the extracted `GDPS_Full_Rater_Source` folder, then run:

```powershell
geode sdk install
geode build
Get-ChildItem -Recurse -Filter *.geode
```

The final command searches for the generated package. A successful `geode build` and an actual `.geode` output are required; do not rename the ZIP to `.geode`.

## Known limitation

This source is an early starter and has not been compiled/tested against your specific setup. The level page button currently shows a list of difficulties; it does not submit a server rating yet.
