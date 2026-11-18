# Arquitectura propuesta

## Baseline existente
- Web modular: `web/index.html`, `web/styles.css`, `web/main.js`, `web/audio.js`, `web/pattern.js`, `web/tb3-midi.js`.
- Web offline: `DFD_ACID_ENGINE_OFFLINE.html`, generado por `build_offline.py`.
- Plugin: `vst3/CMakeLists.txt` y `vst3/Source/*.cpp|*.h`, basado en JUCE.
- FM1: `fm1/acid_core.c/.h`, C portable.

## Dirección recomendada
1. Núcleo de **datos y música**: schema de patrón y versionado, notas + duración + gate + accent + slide + probabilidad + ratchet + microtiming + locks + seed, importación/exportación JSON/MIDI. Añadir migración de estados anteriores. Validador estricto de JSON.
2. Motor DSP consistente. Puede tener implementaciones WebAudio WASM o JS y DSP C++ optimizado; no hace falta identidad de muestras si hay paridad en interpretación de patrones. Evitar prometer cross-platform bit-identical sin pruebas.
3. Engine VST3 real-time safe: no allocations, archivos, SysEx, locks o UI callbacks bloqueantes en `processBlock`; comunicación lock-free o mensajes en hilos seguros; manejo de cambios de tempo, seek, loop, stop y sample rate. Test de sobrecarga y voces.
4. Capa dispositivo TB-3 con protocolos Roland en módulo aislado: `identify`, `readBlock`, `validate`, `snapshot`, `previewChange`, `restoreActivePatch`; no escribir datos arbitrarios.
5. Conexión Web MIDI HTTPS/localhost y en VST3 por MIDI del host o servicio local probado, con límites SysEx visibles. Nunca mezclar audio USB con interfaz MIDI.
6. FM1 es plataforma embebida independiente. Mapeo de botones, pantalla, USB y memoria según firmware SDK elegido y licencia. Si la CPU no soporta todos los FX, priorizar latencia, estabilidad y generación MIDI.
7. Visuales: modo EASY y EXPERT; mantener panel técnico, fuerte jerarquía y tipografía Helvetica con variantes de sistema. Diseño propio que recuerde al espíritu editorial digital/industrial sin copiar propiedad intelectual ajena.

## Estructura objetivo sugerida
```
apps/web/
plugins/vst3/
firmware/fm1/
packages/pattern-schema/
packages/midi-protocol/
presets/
tests/unit/
tests/e2e/
tests/hardware/
docs/
.github/workflows/
```
Migrar solo si mejora claramente la mantenibilidad, preservando git history y código funcional.

## Notas de VST/JUCE
- Verificar CMake, compatibilidad de versión JUCE, requisitos VST3/AU y términos de licencia antes de distribución cerrada o comercial.
- Binarios Mac arm64 y Windows x64: build y firma/notarización sólo si se dispone de identidad legal válida y autorización; nunca inventar certificados.
- Automatización total de controles importantes con `AudioProcessorValueTreeState`; serialization de presets, patrones y parámetros.
- Implementar exports/drag & drop MIDI sin bloquear el hilo de audio.
- Usar pluginval o validación de host y pruebas reales en Ableton. SysEx a través del DAW puede estar filtrado: implementar ruta alternativa documentada cuando haga falta.
