#!/bin/bash
# DFD ACID ENGINE · Mac installer: copies the plug-ins to your user plug-in folders and clears the download quarantine.
cd "$(dirname "$0")"
echo ""
echo "  D F D  /  A C I D   E N G I N E"
echo "  DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO"
echo ""
VST3="$HOME/Library/Audio/Plug-Ins/VST3"; AU="$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$VST3" "$AU" "$HOME/Applications"
rm -rf "$VST3/DFD ACID ENGINE.vst3" "$AU/DFD ACID ENGINE.component" "$HOME/Applications/DFD ACID ENGINE.app"
cp -R "DFD ACID ENGINE.vst3" "$VST3/" && cp -R "DFD ACID ENGINE.component" "$AU/" && cp -R "DFD ACID ENGINE.app" "$HOME/Applications/"
xattr -dr com.apple.quarantine "$VST3/DFD ACID ENGINE.vst3" "$AU/DFD ACID ENGINE.component" "$HOME/Applications/DFD ACID ENGINE.app" 2>/dev/null
killall -9 AudioComponentRegistrar 2>/dev/null
echo "  ✓ VST3  → $VST3"
echo "  ✓ AU    → $AU"
echo "  ✓ App   → ~/Applications"
echo ""
echo "  Ableton Live: Settings → Plug-ins → 'Use Audio Units' / 'Use VST3 Plug-in System Folders' → Rescan."
echo "  Ableton Live: Ajustes → Plug-ins → 'Usar Audio Units' / 'Usar carpetas de sistema VST3' → Volver a escanear."
echo "  Free and open source · Gratis y de código abierto · ♥ PayPal: drophlex@gmail.com"
echo ""
read -n 1 -s -r -p "  Press any key / Presiona una tecla…"
