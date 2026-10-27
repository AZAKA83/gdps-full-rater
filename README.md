# GDPS Full Rater — source starter

This is a starter source project for a Windows Geode mod plus a server-side adapter template. It is **not an installable `.geode` yet** and has not been compiled/tested in this environment (the Geode SDK is not installed here).

## Included

- A level-info-page button that identifies the selected level and lists the built-in and extra demon tiers.
- `difficulties.json`, listing standard Geometry Dash difficulties and extra demon tiers.
- A server-side schema and Express route template for saving custom difficulty data.
- A CMake project and a Windows build helper.

## What is and is not real yet

- Standard GDPS `!rate` ratings can be real if your server supports the command and your account has `commandRate`. The common command accepts the built-in difficulty values (Auto/Easy/Normal/Hard/Harder/Insane/Demon).
- Custom tiers such as Supreme Demon and Grandpa Demon are listed in this starter but **are not submitted by the current C++ UI**. The server adapter and client network call must be connected to your exact GDPS core before custom ratings persist or display for everyone.
- This is therefore a development starter, not a fake claim that custom tiers are already working server-wide.

## Target

- Windows
- Geometry Dash 2.2081
- Geode 5.10.1 SDK

The Geode mod index metadata currently targets 2.2081. Update the `gd.win` value and test the build if your game is a different version, such as 2.209.

## Build on Windows (using Scoop)

Scoop installs the tools from the command line, but it does not replace the C++ compiler/Windows SDK required to compile a Geode mod.

1. Open a **normal (not Administrator)** PowerShell window. Install Scoop from its official installer:

   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
   ```

2. Install Geode CLI and build tools:

   ```powershell
   scoop bucket add extras
   scoop install geode-sdk-cli
   scoop install cmake git ninja llvm
   ```

3. Install **Visual Studio 2022 Build Tools** from https://visualstudio.microsoft.com/downloads/. In the installer select **Desktop development with C++** and make sure the MSVC C++ build tools and a Windows SDK are included. LLVM installed through Scoop still needs the Windows SDK and CRT libraries.

4. Close and reopen PowerShell, open the project folder, then install the Geode SDK:

   ```powershell
   geode sdk install
   ```

5. Build the project from its root folder:

   ```powershell
   geode build
   ```

6. Search the project folder for the output package:

   ```powershell
   Get-ChildItem -Recurse -Filter *.geode
   ```

   Only install it if the build completed successfully and the `.geode` file was actually generated. If the build fails, copy the first CMake/compiler error; this source starter may need adjustment for your installed SDK and game version.

Alternative: install only the Geode CLI with WinGet using `winget install GeodeSDK.GeodeCLI`, as documented at https://docs.geode-sdk.org/getting-started/geode-cli/.

## Why the backend has to be changed

A client-side mod cannot force your GDPS to store custom tiers. The server must authenticate the rating account, check its `commandRate` permission, save the custom tier in its database, and expose that tier when the level information is loaded. Adapt the files in `server-addon/` to your particular GDPS core before deploying.

## Sources used for the architecture

- Geode SDK: https://github.com/geode-sdk/geode
- Rate Button (existing GDPS mod): https://github.com/M336G/RateButton
- GDPS command guide: https://gdpsfh.gitbook.io/community-guide/in-game-help/level-commands

This starter is independent code and is not affiliated with Geometry Dash, RobTop, Geode, or the maintainers of those projects.

## Cvolton GDPS core

A Cvolton-specific PHP/MySQL server addon is included in `server-addon/cvolton/`.
It adds a separate custom-tier table plus authenticated set/public read endpoints.
Import its SQL and copy the two PHP files to the Cvolton server web root, following
that folder's README. The current client source is still a UI starter: its network
calls are not wired to these endpoints yet, so the package is not yet a finished
end-to-end mod and cannot yet make custom ratings persist from the in-game button.
