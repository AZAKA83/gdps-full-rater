# Build GDPS Full Rater online (no local C++ installation)

This project includes a GitHub Actions workflow that builds Android64 and Windows packages in GitHub's cloud runners. It is based on the official Geode SDK build action.

## Steps
1. Create a new repository on GitHub.
2. Upload the **contents of this folder** to the repository root (so `mod.json`, `CMakeLists.txt`, `src/`, and `.github/workflows/build.yml` are at the root).
3. Open the repository's **Actions** tab and enable Actions if prompted.
4. Open **Build GDPS Full Rater** and click **Run workflow**, or wait for the first build triggered by your upload.
5. Open the completed workflow run and download the artifact named `GDPS-Full-Rater-Android64` for Android, or `GDPS-Full-Rater-Windows` for Windows. The downloaded artifact contains the `.geode` package.

Note: this source is a starter prototype. Its in-game button currently displays an information panel; custom ratings are not yet connected to the server/database. The workflow only builds the code provided and cannot make unfinished rating logic functional.
