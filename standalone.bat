@echo off

FOR /F "tokens=* USEBACKQ" %%F IN (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property InstallationPath`) DO (
SET vspath=%%F
)
set VSCMD_START_DIR=%1
echo "InstallationPath: %vspath%"
CALL "%vspath%\VC\Auxiliary\Build\vcvarsall.bat" x64
just _build_standalone
