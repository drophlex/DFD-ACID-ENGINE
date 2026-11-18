# Aceptación y pruebas

## Web
- [ ] El enlace HTTPS público abre la APP renderizada, NO código fuente ni ZIP.
- [ ] Audio PLAY/STOP desde gesto del usuario; no hay notas colgadas.
- [ ] Seed reproducible y mutación musical editable; escalas y presets originales funcionan.
- [ ] MIDI exportado abre en Ableton y conserva tempo/duración/nota, accent/slide acorde a destino.
- [ ] WAV reproduce correctamente, sin clipping accidental y con duración esperada; la exportación no promete capturar TB-3 real.
- [ ] JSON proyectos importados con validación, versiones y gestión de errores.
- [ ] Chrome, Edge, pantallas móviles; sin controles inutilizables; atajos accesibles; audio WebAudio tras gesto.
- [ ] Web MIDI se habilita sólo cuando hay soporte y autorización, no aparece como función disponible en navegadores incompatibles.

## VST3
- [ ] Mac arm64 y Windows x64 builds VST3 reales e instalables, pruebas de carga/escaneo en host compatible.
- [ ] DAW envía MIDI/clock correctamente; audio limpio 44.1/48/96kHz, cambio tempo, loop y stop.
- [ ] Sin asignaciones de memoria, disco, locks ni operaciones SysEx bloqueantes en audio callback.
- [ ] Estados/presets, automation, undo/redos, MIDI output/export importan/exportan sin corrupción.
- [ ] Sin restricciones artificiales de edición/exportación/presets ni paywalls.
- [ ] Test pluginval si disponible, audio silenciado (no NaNs), CPU medida en entorno real.

## TB-3 física
- [ ] Usuario autoriza permisos; puerto y modelo identificados.
- [ ] Lectura de patch validada, checksum y tamaño correctos; snapshot reproducible.
- [ ] Envía CC de cutoff/resonance, slide/accent conforme al manual/firmware real.
- [ ] Envío de SysEx de prueba seguro y restauración del PATCH ACTIVO sólo tras backup y confirmación.
- [ ] Reconexión USB, timeout, cancelación y All Notes Off.
- [ ] Captura MIDI si y sólo si la unidad realmente lo transmite; evidencia de notas.
- [ ] Audio físico capturado en host/DAW mediante ruta de audio apropiada.
- [ ] Sin operaciones de memoria interna no documentadas.

## FM1
- [ ] Compilación con SDK/toolchain correcto y licencia examinada.
- [ ] CPU/RAM y latencia aceptables, sin glitches; teclas/pantalla/MIDI/memoria integrados.
- [ ] Recuperación de firmware original documentada y comprobable.
- [ ] Prueba real con FM1 antes de declarar firmware instalable.

## Marca, calidad y derechos
- [ ] Helvetica/similares y diseño original sofisticado; créditos visibles en todas las versiones.
- [ ] No usar samples, código, sonidos extraídos ni logotipos de terceros sin licencia.
- [ ] Sin enlaces inventados; todo binario adjunto se instaló/probó en alguna plataforma real indicada.
- [ ] Documentar evidencias y limitaciones por versión.
