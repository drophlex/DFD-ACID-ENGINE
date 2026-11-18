# Estado real al entregar

**Proyecto existente:** `PROYECTO_ACTUAL/`. Prototipo original. No borrar las fuentes ni reiniciar por capricho.

## Validaciones ejecutadas en este paquete
- `node --test tests/pattern.test.mjs`: 7 pruebas completadas; 7 aprobadas.
- `node --check web/main.js`, `web/pattern.js`, `web/audio.js`, `web/tb3-midi.js`: sintaxis válida.
- `cc -std=c11 -O2 -Wall -Wextra -c fm1/acid_core.c -o /tmp/acid_core.o`: compilación del núcleo C portable completada.
- Intento de navegador headless local: bloqueado por políticas de acceso del navegador del entorno (`net::ERR_BLOCKED_BY_ADMINISTRATOR`). No se pudo certificar la interacción browser/E2E. NO señalar este control como aprobado.

## Validación pendiente
- No hay una URL pública publicada confirmada.
- No hay builds VST3 confirmados para Mac/Windows; el proyecto es fuente JUCE y puede contener errores de integración.
- No se realizaron pruebas reales de SysEx/MIDI con una Roland TB-3 conectada.
- No se probó captura de audio USB de la TB-3 en DAW.
- El C para FM1 no es firmware flasheable; no se integró ni probó en FM1.
- No se evaluaron aún calidad de sonido, antialiasing, sincronía musical en hosts ni carga CPU.

## Datos de archivos
- `web/`: interfaz por módulos JS/CSS y sintetizador con AudioContext.
- `DFD_ACID_ENGINE_OFFLINE.html`: demo web monolítica. `SITIO_LISTO_PARA_DESPLEGAR/index.html` es copia lista para alojar.
- `vst3/`: CMake/JUCE, DSP y editor fuente.
- `fm1/`: núcleo C portable, guía de integración.
- `tests/`: siete pruebas Node básicas.
- `README_ES.md`, `NOTICE.md`: notas de instalación, atribución, límites y referencias.

**El responsable de la siguiente fase debe mantener este reporte actualizado con evidencia.**
