# DFD / ACID ENGINE
## DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB, MÉRIDA, YUCATÁN

**Estado:** entrega de código fuente y primer prototipo funcional; no es todavía un producto final certificado.

Sintetizador ácido y generador de patrones originales, con secuenciador, edición musical, archivos MIDI/WAV, enlace MIDI USB con Roland TB-3 y proyectos de adaptación VST3/FM1. Creado como software original, sin copiar el código ni los sonidos de otros desarrolladores. Los modelos de referencia son clásicos de la música electrónica, no una promesa de clonación bit a bit.

## 1. Pruébalo sin instalar nada

Abre el archivo **`DFD_ACID_ENGINE_OFFLINE.html`** de la raíz en un navegador moderno. Funciona sin internet para sonido virtual, edición de secuencias y exportación local MIDI/WAV. **Es la forma más fácil de probarlo.**

- `PLAY`: escucha el sintetizador virtual.
- `GENERATE NEW PATTERN`: crea una secuencia original con raíz, escala, densidad y caos.
- `MUTATE CURRENT DNA`: altera una secuencia existente, no elimina toda su identidad.
- En cada paso, pulsa la nota para activar/silenciar, `A` para acento, `S` para slide, `R` para ratchets de 2–4 subdivisiones. Clic derecho sube un semitono; Shift + clic baja uno (en escritorio).
- `EXPORT MIDI`: archivo MIDI tipo 0 que incluye CC 102 para slide y CC 103 para accent, diseñado para importar en Ableton Live o enviarlo a la TB-3.
- `EXPORT WAV`: renderizado local del sintetizador virtual a **48 kHz / 24 bit PCM**, con drive y delay. **No graba el audio de la TB-3 física.**
- `SAVE`, `RECALL` y proyecto JSON: patrones guardados localmente, importables/exportables.

Para USB MIDI/SysEx la recomendación es servir `web/` por **localhost** o por una página **HTTPS**. Web MIDI necesita navegador y permisos compatibles. Un archivo suelto puede tener bloqueado el acceso físico aunque el instrumento virtual funcione.

### Servidor web local

Si tienes Python instalado, desde esta carpeta ejecuta:

```bash
cd web
python3 -m http.server 8000
```

Abre `http://localhost:8000` en Chrome o Edge de escritorio. Puedes desplegar el contenido de `web/` en GitHub Pages, Netlify o tu sitio de Polaris Sound Lab, sin procesador externo. El backend no es necesario. Verifica el soporte de Web MIDI de cada teléfono.

## 2. Roland TB-3 física por USB: TWIN LINK

**Componentes implementados en web:**

- Solicitud de acceso MIDI con opción SysEx y detección de puertos cuya identificación incluya `TB-3`.
- Envío de notas y controles MIDI 102 (slide) / 103 (accent) en modo **TWIN** (Roland + virtual) o **TB-3 ONLY**. Por seguridad, inicialmente el envío a hardware está **OFF**.
- MIDI input **SHADOW CAPTURE**: captura notas transmitidas por la TB-3, cuantiza a la grilla de 1/16 y permite mutar/exportar el nuevo patrón. Depende de que la TB-3 transmita notas por el puerto/configuración seleccionados.
- Controles en vivo CC 74 cutoff y CC 71 resonance. Algunas funciones también contemplan CC 68/69 Scatter y 12/13 Env Mod.
- **Identificación de equipo** SysEx de Roland.
- **Lectura SysEx oficial** de bloques `PATCH`, `DISTORTION`, `EFX1` y `EFX2`; exportación de snapshot JSON de datos recibidos.
- **Restauración manual y confirmada** de snapshots en los parámetros del patch ACTIVO (no escritura automática a la memoria de usuario).
- Ajustes directos de mezcla de osciladores, decay, tipo de efecto y hasta 12 parámetros genéricos de efectos. Los significados dependen del efecto seleccionado: consulta la tabla oficial de Roland antes de editar valores.

La aplicación utiliza las direcciones, tamaños de bloques, device ID y checksum Roland documentados en el archivo `TB-3_MI_1.pdf`. Todo cambio de firmware/sistema puede alterar compatibilidad. **No se ha conectado ni probado todavía con tu TB-3 real.** Haz una copia de seguridad original antes de restaurar un patch.

### Conexión recomendada a Ableton Live

1. Conecta la TB-3 al Mac con USB tipo B; revisa el driver oficial compatible con tu macOS y confirma la presencia del dispositivo MIDI.
2. Configura la TB-3 para recibir MIDI por el canal elegido, y evita ejecutar un patrón en su secuenciador a la vez que envías notas externas si no quieres escuchar ambos.
3. En Chrome/Edge, abre la página local y pulsa **CONNECT USB MIDI**, concede permiso SysEx si quieres usar el editor profundo. La opción de transmisión comienza en `OFF`.
4. Para capturar notas de la TB-3, pulsa **START RECORDING**, reproduce un patrón en la Roland y después **STOP + QUANTIZE**. Si no hay notas, comprueba que la TB-3 está transmitiendo MIDI; la aplicación no lee datos de patrones propietarios guardados por el modo USB de mantenimiento.
5. En modo TWIN, el navegador sintetiza una capa virtual y envía la secuencia MIDI a la Roland. Su sonido real entra por audio USB (si tu sistema/driver lo permite) o por las salidas analógicas a tu interfaz; grábalo en una pista de audio de Ableton. No confundir audio USB con Web MIDI.
6. Si utilizas la versión VST3, **el DAW debe enrutar la salida MIDI del instrumento al puerto TB-3**. No todos los hosts dejan pasar SysEx desde plugins. Si tu host lo bloquea, usa el panel web para las operaciones SysEx.

### Qué NO hace todavía

- No lee el contenido musical de un patrón interno por SysEx no documentado ni guarda patrones de forma remota en los slots de la TB-3.
- No hace captura automática de audio USB en el navegador. El renderizado WAV procede del sintetizador virtual, no del hardware.
- No se ha validado la latencia MIDI ni la estabilidad SysEx con una Roland TB-3 física. Puede ser necesaria calibración de sincronización.
- No replica al 100 % la circuitería original TB-303 ni todas las pantallas/controles del editor Dope Robot. Es un desarrollo original inspirado en las capacidades de edición documentadas.

## 3. VST3 nativo / JUCE

En `vst3/` hay un proyecto CMake JUCE 8.0.17-security con **VST3 + aplicación standalone**. Incluye: instrumento de síntesis mono, secuenciador que sigue el BPM/transport del DAW, exportación MIDI, generador/mutador, estados de patch dentro del proyecto, parámetros automatizables, editor visual propio, salida de notas y CC a hardware, y comandos SysEx activos para Patch/Distortion/FX1/FX2 compatibles con el mapa oficial Roland.

Los patrones VST3 se guardan como **vectores dinámicos** y la interfaz tiene paginación de 16 notas; no hay límite comercial de longitud, cantidad de presets guardados por el usuario, licencias de exportación ni suscripciones. Los límites reales son la memoria del sistema y las capacidades del host/plugin. No se han implementado todas las opciones de sonido y efectos deseadas en esta primera versión.

**Para compilar (requiere internet, SDK y compilador):**

```bash
cmake -S vst3 -B build/vst3 -DCMAKE_BUILD_TYPE=Release
cmake --build build/vst3 --config Release -j4
```

CMake descargará JUCE desde su repositorio; dependiendo del sistema necesitas Xcode/CLT o Visual Studio. No se incluye un `.vst3` binario porque no se compiló ni probó en un entorno Mac/Windows con JUCE. Algunos DAW no enrutan SysEx emitido por VST3. El editor VST está diseñado como panel nativo JUCE: comparte identidad visual pero no es idéntico a la vista web.

**Licencia JUCE:** JUCE se distribuye con términos de GPL/comerciales que debes revisar antes de distribuir un plugin binario, especialmente si será cerrado o se venderá. Este proyecto no incorpora JUCE ni el código de Dope Robot en el ZIP.

## 4. M-VAVE FM1

`fm1/acid_core.c` es código C portátil y compilable, con oscilador, sub, filtro, overdrive, envolvente, slide, secuenciador y salida MIDI por callback. **Aún NO es un firmware `.fwsc` listo para flashear.** Falta integrarlo con los controladores y el SDK específico del FM1. Está documentado en `fm1/README.md`.

Si el FM1 está ejecutando firmware capaz de actuar como USB MIDI, ya puedes usarlo como controlador del proyecto web, según la compatibilidad y mapeo MIDI del firmware instalado. Eso es distinto de ejecutar nativamente el motor de DFD dentro del FM1.

## 5. Pruebas y seguridad

```bash
node --test tests/pattern.test.mjs
cc -std=c11 -O2 -Wall -Wextra -c fm1/acid_core.c -o /tmp/acid_core.o
```

Pruebas incluidas: generación determinista, exportación de MIDI válido, codificación VLQ, checksums SysEx Roland y WAV PCM 24 bit. El código JavaScript ha pasado una revisión sintáctica. Las pruebas automáticas **no sustituyen** pruebas musicales, de USB MIDI, ni un build VST3 real.

## 6. Fuentes técnicas (referencias; no código copiado)

- Roland TB-3 MIDI Implementation v1.10: https://cdn.roland.com/assets/media/pdf/TB-3_MI_1.pdf
- Roland TB-3 v1.10 update: https://cdn.roland.com/assets/media/pdf/TB-3_v110_e.pdf
- Roland Support: https://www.roland.com/global/products/tb-3/support/
- Dope Robot TB-3 resources/editor: https://www.doperobot.com/tb3.html
- M-VAVE FM1 Open SDK: https://github.com/m-vave/fm1_open
- JUCE framework: https://github.com/juce-framework/JUCE

## Autoría y propiedad intelectual

**DARK FUNK DURCAL DESIGN. DESARROLLADO EN POLARIS SOUND LAB, MÉRIDA, YUCATÁN, MÉXICO.** No afiliado a Roland, Dope Robot, JUCE o M-VAVE. Los nombres se mencionan únicamente para describir compatibilidad y referencias técnicas.
