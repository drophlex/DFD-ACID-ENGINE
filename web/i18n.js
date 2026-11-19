// DFD ACID ENGINE · English (default) and Spanish. The credit line is never translated.
export const T = {
  en: {},
  es: {
    easy: 'FÁCIL', expert: 'EXPERTO', engineIdle: 'MOTOR LISTO', engineOn: 'SONANDO',
    eyebrow: '/ 001 · EL SISTEMA DE GENERACIÓN ÁCIDA', tagline: 'SECUENCIADOR / SINTETIZADOR / LABORATORIO DE PATRONES / GEMELO DE HARDWARE',
    heroPlay: '▶ ENCENDER LA MÁQUINA', heroHint: 'Usa audífonos. Espacio = play / stop · teclas Z–M = tocar notas',
    play: 'PLAY', stop: '■ STOP', tempo: 'TEMPO', steps: 'PASOS', swing: 'SWING', editing: 'EDITAR', quality: 'CALIDAD',
    exportMidi: '↓ MIDI', dragMidi: '⇲ ARRASTRA MIDI', exportWav: '↓ WAV', exportStems: '↓ STEMS', panic: 'PÁNICO',
    genHead: '01 / GENERADOR', genTitle: 'CAOS<br><em>HUMANO.</em>', archetype: 'ESTILO', key: 'TONO', scale: 'ESCALA', density: 'DENSIDAD', chaos: 'CAOS', mutation: 'MUTACIÓN',
    gProb: 'PROBABILIDAD', gCond: 'CONDICIONES', gGate: 'VARIACIÓN DE GATE', poly: 'POLIRRITMO', transpose: 'TRANSPONER',
    dnaLock: 'BLOQUEO DE ADN', lNotes: 'NOTAS', lRhythm: 'RITMO', lAcc: 'ACENTOS', lSlide: 'SLIDES',
    generate: '↗ GENERAR PATRÓN NUEVO', mutate: '↻ MUTAR EL ADN ACTUAL', copyAB: '⇉ COPIAR A → B',
    genTip: 'NOTA / Los patrones se generan en tu navegador, no en un servidor. Bloquea genes y genera: solo cambian los que no están bloqueados.',
    seqHead: '02 / MATRIZ DE PASOS', sequence: 'SECUENCIA', seqHelp: 'CLIC = ON/OFF · CLIC DERECHO / SHIFT = TONO ± · A ACENTO · S SLIDE · T LIGADURA · R RATCHET · NÚMERO = INSPECCIONAR',
    clear: 'BORRAR', shuffle: 'REVOLVER', inspHead: 'INSPECTOR DE PASO', prob: 'PROBABILIDAD', cond: 'CONDICIÓN',
    sndHead: '03 / DISEÑO DE SONIDO', memHead: '04 / MEMORIA', archTitle: 'EL<br><em>ARCHIVO.</em>', factory: 'ADN DE FÁBRICA', loadPreset: 'CARGAR PATRÓN DE FÁBRICA',
    yourName: 'NOMBRE DE TU PATRÓN', saveLocal: 'GUARDAR EN ESTE NAVEGADOR', recall: 'RECUPERAR PATRÓN GUARDADO', exportProject: '↓ DESCARGAR PROYECTO .JSON', importProject: '↑ IMPORTAR PROYECTO .JSON',
    activeNotes: 'NOTAS ACTIVAS', filterNow: 'FILTRO AHORA', free: 'Libre y de código abierto (GPL-3.0). Sin cuenta, sin suscripción, sin límites de exportación.',
    fxHead: '05 / RACK DE EFECTOS', fxSub: 'SINCRONIZADO AL TEMPO · CADENA: CRUSH → RING → STUTTER → FILTRO DIN → MOD → COMP → DELAY → REVERB',
    perfHead: '06 / EN VIVO', perfSub: 'MANTÉN LOS PADS · ARRASTRA EL XY · TOCA LAS TECLAS',
    padFill: 'FILL<small>mantener</small>', padStut: 'STUTTER<small>mantener</small>', padMute: 'MUTE<small>alternar</small>', padAcid: 'SQUELCH<small>mantener</small>',
    hwTag: 'UN PATRÓN. DOS MÁQUINAS. ACCIDENTES INFINITOS.', hwConn: '01 / CONEXIÓN', connect: 'CONECTAR USB MIDI', demo: 'DEMO: TB-3 SIMULADA (sin hardware)',
    hwNote: 'USB MIDI ≠ USB AUDIO. Graba el audio real de la TB-3 con tu interfaz o tu DAW.',
    capText: 'Toca un patrón en tu TB-3 y captura sus notas. Se cuantizan en la matriz, listas para mutar y devolver.', capStart: '● EMPEZAR A GRABAR', capStop: '■ DETENER + CUANTIZAR',
    sysText: 'Lee los bloques documentados del patch activo. Toda escritura te pregunta antes; restaurar siempre hace un respaldo antes de escribir.',
    identify: 'IDENTIFICAR TB-3', readPatch: 'LEER PATCH ACTIVO', savePatch: '↓ GUARDAR RESPALDO .JSON', loadPatch: '↑ CARGAR RESPALDO', restore: 'RESTAURAR PATCH ACTIVO (CONFIRMAR)',
    applyType: 'APLICAR TIPO DE FX', applyParam: 'APLICAR PARÁMETRO', restoreNote: 'Restaurar solo cambia el patch activo (de edición), nunca un slot de usuario guardado. Nada se escribe sin tu clic.',
    notAff: 'SOFTWARE INDEPENDIENTE · SIN AFILIACIÓN CON ROLAND', donate: '♥ DONAR',
  },
};

// parameter labels (EN from the engine's table, ES here)
export const PNAME_ES = {
  tune: 'Afinación', oct: 'Octava', wave: 'Onda', pw: 'Ancho de pulso', sub: 'Sub', noise: 'Ruido', cutoff: 'Corte', res: 'Resonancia', envmod: 'Env Mod', decay: 'Caída', accent: 'Acento', glide: 'Tiempo de slide',
  drive: 'Drive', drive2: 'Overdrive', tone: 'Tono', vcadecay: 'Caída de amp', crush_bits: 'Bits', crush_rate: 'Frecuencia', crush_mix: 'Mezcla', ring_freq: 'Frecuencia', ring_mix: 'Mezcla', stut_len: 'Largo',
  dyn_rate: 'Ciclo', dyn_depth: 'Profundidad', dyn_res: 'Resonancia', mod_type: 'Tipo', mod_rate: 'Velocidad', mod_depth: 'Profundidad', mod_fb: 'Realimentación', mod_mix: 'Mezcla',
  comp_thr: 'Umbral', comp_ratio: 'Ratio', comp_make: 'Ganancia', dly_time: 'Tiempo', dly_fb: 'Realimentación', dly_tone: 'Tono', dly_mix: 'Mezcla', dly_ping: 'Ping-pong',
  rev_size: 'Tamaño', rev_damp: 'Amortiguación', rev_mix: 'Mezcla', volume: 'Volumen', human: 'Humanizar',
};
export const PNAME_EN = {crush_bits: 'Bits', crush_rate: 'Rate', crush_mix: 'Mix', ring_freq: 'Frequency', ring_mix: 'Mix', stut_len: 'Length', dyn_rate: 'Cycle', dyn_depth: 'Depth', dyn_res: 'Resonance',
  mod_type: 'Type', mod_rate: 'Rate', mod_depth: 'Depth', mod_fb: 'Feedback', mod_mix: 'Mix', comp_thr: 'Threshold', comp_ratio: 'Ratio', comp_make: 'Makeup', dly_time: 'Time', dly_fb: 'Feedback', dly_tone: 'Tone', dly_mix: 'Mix', dly_ping: 'Ping-pong', rev_size: 'Size', rev_damp: 'Damping', rev_mix: 'Mix'};

let lang = 'en';
export const getLang = () => lang;
export function applyLang(l) {
  lang = l === 'es' ? 'es' : 'en';
  document.documentElement.lang = lang;
  const d = T[lang];
  document.querySelectorAll('[data-i18n]').forEach(el => { if (el.dataset.en === undefined) el.dataset.en = el.innerHTML; el.innerHTML = d[el.dataset.i18n] ?? el.dataset.en; });
  document.querySelectorAll('[data-i18n-html]').forEach(el => { if (el.dataset.en === undefined) el.dataset.en = el.innerHTML; el.innerHTML = d[el.dataset.i18nHtml] ?? el.dataset.en; });
  try { localStorage.setItem('dfd-acid-lang', lang); } catch {}
}
export const tr = (key, en) => (T[lang][key] ?? en);
