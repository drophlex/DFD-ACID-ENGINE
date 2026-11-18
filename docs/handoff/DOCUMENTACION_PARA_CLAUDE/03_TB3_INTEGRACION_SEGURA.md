# TB-3 USB / TWIN LINK / SysEx seguro

## Fuentes principales
- Manual de implementación MIDI Roland: https://cdn.roland.com/assets/media/pdf/TB-3_MI_1.pdf
- Notas de firmware Roland v1.10: https://cdn.roland.com/assets/media/pdf/TB-3_v110_e.pdf
- Soporte Roland TB-3: https://www.roland.com/global/products/tb-3/support/
- Recursos de Dope Robot (estudio de funcionalidades, NO para copiar código): https://www.doperobot.com/tb3.html

## Reglas técnicas
1. Identificar físicamente puerto TB-3 por nombre y respuesta SysEx compatible cuando sea posible. Si hay varios puertos iguales, pedir selección explícita. No enviar SysEx a otro dispositivo por defecto.
2. Web MIDI requiere origen HTTPS/localhost, navegador compatible y permiso SysEx del usuario. No prometer soporte universal en Android/iOS.
3. Cable USB permite datos MIDI; **Web MIDI NO da acceso al audio**. Audio del TB-3 se capta por driver/interfaz de audio y DAW. Si se ofrece audio web por MediaDevices es un flujo independiente con permiso y compatibilidad verificados.
4. Marcar claramente OFF / VIRTUAL / TB-3 ONLY / TWIN. Salida hardware comienza siempre apagada; incluir PANIC (CC123 / note off de todas las notas activas).
5. Distinguir lo documentado por Roland de inferencias experimentales. Solo cambios de patch ACTIVO después de backup/snapshot; no borrar ni escribir bancos internos no documentados.
6. SysEx: comprobar manufacturer ID, model ID, unit ID, comando RQ1/DT1, dirección, tamaño, checksum Roland y longitud recibida; validez de cada byte de datos (0–127). Evitar restauraciones parciales silenciosas.
7. Test de ida/vuelta lectura y restauración en unidad real antes de habilitar botones de escritura predeterminados. Eliminar loops de retroalimentación MIDI/monitoring.
8. Lista desplegable de parámetro **por cada tipo de efecto**, nombre, rango y unidad, con validación contra el manual. No presentar `PARAMETER 3` como si fuera un control universal equivalente.
9. Añadir `read timeout`, retry limitado y estado de enlace. No mandar ráfagas simultáneas de RQ1/DT1. Registro de operaciones exportable sin datos privados.
10. Distinguir ``patch librarian`` compatible de ``pattern memory librarian``. No prometer lectura/carga de los patrones internos si no está documentada o probada.

## Casos de prueba con hardware físico
- Identificación y reconexión después de desenchufar/reconectar USB.
- Solicitar snapshot del patch ACTIVO; corroborar longitudes y checksum en todas las respuestas.
- Cambiar cutoff/resonancia con CC; escuchar cambios; A/B con valores originales.
- Después de backup, escribir UN parámetro de patch ACTIVO, leer de vuelta y restaurar valor original.
- Crear 16 pasos con slide/accent; comprobar comportamiento con secuenciador interno parado y distintos canales MIDI.
- Capturar notas MIDI emitidas por TB-3, si hay emisión; si no, mostrar explicación accionable en vez de inventar datos.
- Probar sincronización por clock/transport y audio en Ableton; medir desfase sin inventar milisegundos.
- Verificar pánico / all notes off y que el cierre del plugin/navegador no deja notas sostenidas.

## Dope Robot
Estudiar las posibilidades de patch editor, parámetros ocultos, efectos y library management como inspiración funcional. El código, apariencia, assets y guías que tengan licencia deben tratarse conforme a sus términos: desarrollar una implementación propia a partir de las especificaciones oficiales y pruebas.
