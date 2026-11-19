#!/bin/bash
# DFD ACID ENGINE · one-line installer for Mac (VST3 · AU · app):
#   curl -fsSL https://drophlex.github.io/DFD-ACID-ENGINE/install.sh | bash
# It downloads the newest plug-in release from GitHub, copies it to your user plug-in folders and clears the download
# quarantine (files fetched with curl are not blocked by Gatekeeper). Nothing is installed outside your home folder.
set -e
echo ""; echo "  D F D  /  A C I D   E N G I N E  ·  installing / instalando…"; echo ""
API="https://api.github.com/repos/drophlex/DFD-ACID-ENGINE/releases?per_page=30"
URL=$(curl -fsSL "$API" | grep -o '"browser_download_url": *"[^"]*/plugin-v[^"]*/DFD-ACID-ENGINE-Mac.zip"' | head -1 | sed 's/.*"\(https[^"]*\)"$/\1/')
if [ -z "$URL" ]; then echo "  ✕ No plug-in release found / No se encontró el plug-in: https://github.com/drophlex/DFD-ACID-ENGINE/releases"; exit 1; fi
echo "  ↓ $URL"
T="$(mktemp -d)"; cd "$T"
curl -fsSL -o D.zip "$URL"
ditto -x -k D.zip .
S="$T/DFD ACID ENGINE (Mac)"
V="$HOME/Library/Audio/Plug-Ins/VST3"; C="$HOME/Library/Audio/Plug-Ins/Components"; mkdir -p "$V" "$C" "$HOME/Applications"
rm -rf "$V/DFD ACID ENGINE.vst3" "$C/DFD ACID ENGINE.component" "$HOME/Applications/DFD ACID ENGINE.app"
cp -R "$S/DFD ACID ENGINE.vst3" "$V/"; cp -R "$S/DFD ACID ENGINE.component" "$C/"; cp -R "$S/DFD ACID ENGINE.app" "$HOME/Applications/"
xattr -dr com.apple.quarantine "$V/DFD ACID ENGINE.vst3" "$C/DFD ACID ENGINE.component" "$HOME/Applications/DFD ACID ENGINE.app" 2>/dev/null || true
killall -9 AudioComponentRegistrar 2>/dev/null || true
cd /; rm -rf "$T"
echo "  ✓ VST3 → ~/Library/Audio/Plug-Ins/VST3/DFD ACID ENGINE.vst3"
echo "  ✓ AU   → ~/Library/Audio/Plug-Ins/Components/DFD ACID ENGINE.component"
echo "  ✓ App  → ~/Applications/DFD ACID ENGINE.app"
echo ""
echo "  Ableton Live: Settings → Plug-ins → Use Audio Units / Use VST3 Plug-in System Folders → Rescan."
echo "  Ableton Live: Ajustes → Plug-ins → Usar Audio Units / Usar carpetas de sistema VST3 → Volver a escanear."
echo "  DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO · ♥ PayPal: drophlex@gmail.com"; echo ""
