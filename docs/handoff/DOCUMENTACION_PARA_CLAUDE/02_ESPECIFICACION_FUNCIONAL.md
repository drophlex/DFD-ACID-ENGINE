# Especificación de producto | DFD ACID ENGINE

## Principios
1. Un minuto para comenzar a tocar, profundidad suficiente para uso profesional.
2. Sintetizador y secuenciador musicales originales, no un clon que copia propiedad intelectual.
3. Exportación real y sin restricciones artificiales de sonidos, patrones y MIDI.
4. VST3: edición completa sin niveles de pago, watermarks o bloqueos; recursos limitados sólo por CPU/RAM/host.
5. Web: lo más completo posible; M-VAVE FM1: adaptar a recursos reales sin fingir que corre un VST.

## Tabla de funciones y alcance
| Área | Web (objetivo) | VST3 (objetivo) | FM1 (objetivo) |
|---|---|---|---|
| Motor acid | AudioContext/WebAudio probado | DSP nativo de alta calidad, automatizable | DSP C optimizado para CPU disponible |
| Generación patrones | SÍ, UI fácil y avanzada | SÍ, sin limitación comercial, optimizado RT | SÍ, capacidad acorde a hardware |
| Exportar MIDI | SÍ, `.mid`, tempo/CC | SÍ + host MIDI + drag-out donde sea viable | USB MIDI export según SDK |
| Exportar audio | WAV virtual generado | Bounces por DAW + render dentro del plugin si viable | Captura a DAW o memoria si posible |
| Presets | Local + JSON | Browser editable, bancos ilimitados por diseño | Flash/SD según soporte real |
| Hardware TB-3 USB | Web MIDI/puertos soportados | MIDI/SysEx a través DAW o helper seguro | Control MIDI sólo si soporta USB host/puerto |
| Editar TB-3 | SysEx documentado + librería | Editor profundo + backup/restore | Opcional según recursos físicos |
| Multiefectos | FX modernos con buen rendimiento | FX completos + mod matrix | FX priorizados por DSP |
| UI | Móvil/desktop, responsive | JUCE panel escalable en macOS/Windows | Pantalla/controles del FM1 |

## 3 niveles de juego musical
- **EASY:** Play / Stop / Generate / Mutate / Export / 6 presets DFD, Tempo / Cutoff / Resonance / Drive / Delay, MIDI out seguro.
- **PRO:** Sequencer: 16–N pasos dinámicos, paginación, notas, gate, ties, slides, accents, swing, ratchets, probability, conditions, microtiming, escala y lock de notas.
- **EXPERIMENT:** Mod Matrix, multi-LFO, velocity morph, scene changes, generador de variaciones A/B, polimetría, performance macro XY, random seed reproducible, slicing/stutter e FX de carácter.

## Sonido
- Osciladores con formas saw/square y sub, mezcla, afinación/octavas; implementación band-limited; filtrado resonante estable; feedback/drive, envelopes amp/filter, accent y slide continuos.
- Efectos de carácter: saturation, bit crush, compressor, ring mod, chorus, phaser, flanger, sync delay, reverberación, glitch/stutter. FX deben poder desactivarse, automatizarse y no generar audio inestable.
- Modo A/B de sonido virtual y posibilidad de duplicar audio analógico/USB de TB-3 en DAW.

## Presets originales sugeridos
`DFD / NIGHT SIGNAL`, `POLARIS / GLASS TEETH`, `DFD / REACTION 04`, `DARK FUNK / VIRUS`, `MERIDA / SUBSYSTEM`, `POLARIS / STRANGE LOOP`, `DFD / BLACKOUT MUTATION`, `POLARIS / LOW EARTH`, `DFD / CARRIER LOST`.
No usar canciones ni samples con copyright de Aphex Twin, Squarepusher, The Prodigy, Roland o Dope Robot. Los géneros son guía de exploración, no una promesa de reproducir obras.

## Lo que más valor ofrece a un dueño de TB-3
- Editor/librarian de parámetros profundos que pueden ser difíciles de alcanzar desde el panel físico.
- Guardar y comparar snapshots de patches, con copia de seguridad legible antes de cambios.
- Secuencias generadas externamente, slides y acentos correctos, sincronizados con Ableton.
- Capturar MIDI saliente cuando el aparato realmente lo transmite, cuantizar y generar variaciones creativas.
- Doble capa virtual+física, con controles de compensación de latencia y procesamiento en DAW.
- Modo PERFORMANCE con secuencias que responden al patrón físico, transposiciones, mutación y Morph, siempre sin afirmaciones de acceso a memoria/funciones no documentadas.
