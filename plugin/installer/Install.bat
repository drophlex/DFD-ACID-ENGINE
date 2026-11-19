@echo off
REM DFD ACID ENGINE - Windows installer (Polaris Sound Lab, Merida, Yucatan, Mexico)
REM Copies the VST3 to the system VST3 folder (needs administrator).
net session >nul 2>&1
if %errorlevel% neq 0 (
  echo Asking for administrator rights / Pidiendo permiso de administrador...
  powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b
)
set "SRC=%~dp0"
echo.
echo  DFD / ACID ENGINE
echo  Installing / Instalando...
xcopy /E /I /Y /Q "%SRC%DFD ACID ENGINE.vst3" "%CommonProgramFiles%\VST3\DFD ACID ENGINE.vst3" >nul
echo.
echo  DONE / LISTO
echo  VST3: %CommonProgramFiles%\VST3\DFD ACID ENGINE.vst3
echo  Open your DAW and rescan: Polaris Sound Lab - DFD ACID ENGINE
echo  Abre tu DAW y vuelve a escanear: Polaris Sound Lab - DFD ACID ENGINE
echo  The standalone app is in this folder / La app independiente esta en esta carpeta: DFD ACID ENGINE.exe
echo  Free and open source. Donate (PayPal): drophlex@gmail.com
echo.
pause
