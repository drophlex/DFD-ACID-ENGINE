# Port del sintetizador al M-VAVE FM1

## Archivos de partida
`PROYECTO_ACTUAL/fm1/acid_core.c`, `acid_core.h`, `README.md`. Código C portable, no firmware.

## Procedimiento exigido a Claude
1. Inspeccionar versión real de FM1 del usuario, hardware, memoria, CPU, tamaño de displays, botones, firmware instalado, serial/modelo cuando sea técnicamente necesario. No solicitar IDs personales que no hagan falta.
2. Elegir SDK abierto real y versión estable (ej. m-vave/fm1_open) y revisar licencia/toolchain/documentación. NO adivinar ABI ni encapsular bytes arbitrarios en un `.fwsc`.
3. Definir interfaz/audio callback, MIDI interno/USB, pantalla, encoders, preset flash y transport. Hacer adaptador a núcleo portable; medir CPU/RAM.
4. Construir desde fuente (ideal CI simulador + toolchain reproducible), guardar logs. Etiquetar salidas `experimental / unverified hardware` hasta prueba física.
5. Respaldar proyectos y voices y firmware original antes de actualizar. Procedimiento seguro de rollback documentado y probado según plataforma.
6. Ninguna imagen final se publica como instalable sin compatibilidad con modelo/firmware exacto y validación de usuario.

## Puntos técnicos a no confundir
FM1 NO ejecuta plugins VST3. La versión FM1 puede priorizar secuenciador/generador MIDI y subset de síntesis; VST3 lleva todo el motor y todos los FX. La limitación de hardware no es un límite comercial.
