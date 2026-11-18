# Plan de ejecución para Claude

## P0 — Entender y proteger lo existente
- [ ] Leer código, docs, NOTICE y versiones. Ejecutar `node --test`, `node --check`, compilación C.
- [ ] Crear `CHANGELOG.md`, tabla de estado por componente y lista de defectos.
- [ ] No borrar, simplificar ni reemplazar todo con mockups.

## P1 — URL REAL Y WEB USABLE
- [ ] Publicar sitio estático desde `SITIO_LISTO_PARA_DESPLEGAR` en hosting autorizado.
- [ ] Abrir URL pública real y comprobar consola sin errores, UI visible, generación, playback, export MIDI y WAV.
- [ ] Añadir test E2E automatizado, UX móvil responsive y mensajes de limitaciones Web MIDI.
- [ ] Actualizar link real en README; no poner URL inventada.

## P2 — VST3 PLENO
- [ ] Compilar CMake+JUCE en macOS arm64 y Windows x64; resolver errores.
- [ ] Validar `processBlock` real-time safe, monophonic analog-inspired DSP con bandlimited oscillators.
- [ ] Mejorar sincronización con host, seek/loop, MIDI IO, automation y estados/presets.
- [ ] Engine de patrones de longitud dinámica, edición por páginas, MIDI drag/export y render confiable.
- [ ] Modo sencillo / profundo y matrices de modulación/FX.
- [ ] Probar VST3 en host real y pluginval (si hay entorno). Empaquetar binarios con versionado.

## P3 — TB-3 PROFUNDA Y TWIN LINK
- [ ] Implementar MIDI USB, SysEx documentado, snapshot de patches, naming de cada parámetro/FX.
- [ ] Revisar protocolo con trazas y banco de pruebas simulado (mock MIDI).
- [ ] Probar con TB-3 física por USB: identidad, lectura/backup, cambio seguro de parámetro, restore, sync, capture, audio por DAW.
- [ ] Implementar modo PERFORMANCE: patrones A/B, variación, doble capa, compensación de latencia.

## P4 — FM1 REAL
- [ ] Verificar plataforma y firmware instalado, SDK actual, tipo de toolchain, memoria y licencia.
- [ ] Integrar núcleo C de FM1 con audio y MIDI de dispositivo, UI hardware, preset memory, optimizar RAM/CPU.
- [ ] Build reproducible, simulador o pruebas unitarias en host, luego prueba física controlada.
- [ ] Guía de backup/restore; nunca instalar imagen no verificada.

## P5 — RELEASE
- [ ] Pruebas audio, MIDI, hardware, licencias y empaquetados.
- [ ] Manual ES/EN, video tutorial opcional, registro de versiones, instaladores reales, releases.
- [ ] Sitio Polaris Sound Lab integrado, analytics opcionales sin rastreo invasivo.

**Cada fase debe cerrar con artefactos, comandos de prueba, evidencia y una lista honesta de pendientes.**
