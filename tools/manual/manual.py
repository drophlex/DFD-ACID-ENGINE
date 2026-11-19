#!/usr/bin/env python3
# DFD ACID ENGINE · web user manual. English edition, Spanish edition and both together.
# python3 tools/manual/shots.py && python3 tools/manual/manual.py   ->  manual/*.pdf
import os, sys, asyncio
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
BUILD = os.path.join(ROOT, 'build', 'manual'); OUT = os.path.join(ROOT, 'manual')
os.makedirs(OUT, exist_ok=True)
L = 'en'
def T(en, es): return es if L == 'es' else en
WEB = 'https://drophlex.github.io/DFD-ACID-ENGINE/'
APP = WEB + 'web/'
REPO = 'https://github.com/drophlex/DFD-ACID-ENGINE'
DON = 'https://www.paypal.com/donate/?business=drophlex%40gmail.com&amp;currency_code=MXN'
IG, YT = 'https://www.instagram.com/polaris.sound.lab/', 'https://www.youtube.com/@PolarisSoundLab'
CREDIT = 'DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO'

def shot(n): return f'shots/{L}_{n}.png'
def img(n, cap='', w='100%', mh=None):
    st = f' style="max-height:{mh};object-fit:cover;object-position:top"' if mh else ''
    return f'<figure style="width:{w}"><img src="{shot(n)}"{st}>' + (f'<figcaption>{cap}</figcaption>' if cap else '') + '</figure>'
def table(head, rows, cls=''):
    return f'<table class="{cls}"><tr>' + ''.join(f'<th>{h}</th>' for h in head) + '</tr>' + ''.join('<tr>' + ''.join(f'<td>{c}</td>' for c in r) + '</tr>' for r in rows) + '</table>'
def k(t): return f'<span class="k">{t}</span>'
def tip(t): return f'<div class="tip"><b>{T("TIP", "CONSEJO")}</b>{t}</div>'
def warn(t): return f'<div class="tip warn"><b>{T("SAFETY", "SEGURIDAD")}</b>{t}</div>'
def head(num, kick, title): return f'<div class="sec"><span class="num">{num}</span><span class="kick">{kick}</span></div><h1>{title}</h1>'

PAGES = []
def page(title, body, cls=''): PAGES.append((cls, body, title))

def cover():
    page('', f'''<div class="cover">
<div class="cmark"><span>D</span><span>F</span><span>D</span></div>
<div class="cline">{T('USER MANUAL', 'MANUAL DE USO')} · {EDITION_NAME} · v{EDITION_VER}</div>
<div class="ctitle">ACID<br><em>ENGINE</em></div>
<div class="csub">{T('SEQUENCER / SYNTHESIZER / PATTERN LAB / HARDWARE TWIN', 'SECUENCIADOR / SINTETIZADOR / LABORATORIO DE PATRONES / GEMELO DE HARDWARE')}</div>
<div class="corbit a"></div><div class="corbit b"></div><div class="corbit c"></div>
<div class="clang">{'ENGLISH · ESPAÑOL' if BOTH else T('ENGLISH EDITION', 'EDICIÓN EN ESPAÑOL')}</div>
<div class="cfoot">{CREDIT}<br><a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a> · <a href="{DON}">♥ {T('Donate', 'Donar')}</a></div></div>''', 'coverp')

def build_web():
    global PAGES; PAGES = []
    # ------------------------------------------------------------ cover
    cover()

    # ------------------------------------------------------------ welcome
    toc = [T('Start in five minutes', 'Empieza en cinco minutos'), T('The screen', 'La pantalla'), T('Toolbar and transport', 'Barra y transporte'), T('The generator', 'El generador'),
           T('The step matrix', 'La matriz de pasos'), T('Step inspector and conditions', 'Inspector de paso y condiciones'), T('Sound design', 'Diseño de sonido'), T('FX rack', 'Rack de efectos'), T('DIRTY LAB', 'DIRTY LAB'),
           T('Perform live', 'Tocar en vivo'), T('Archive and exports', 'Archivo y exportación'), T('With Ableton Live', 'Con Ableton Live'), T('TB-3 TWIN LINK', 'TB-3 TWIN LINK'),
           T('Troubleshooting', 'Solución de problemas'), T('Recipes', 'Recetas'), T('Credits and support', 'Créditos y apoyo')]
    page(T('Welcome', 'Bienvenida'), f'''{head('00', T('WELCOME', 'BIENVENIDA'), T('Acid lines you would <em>actually use.</em>', 'Líneas ácidas que <em>sí usarías.</em>'))}
<p class="lead">{T('ACID ENGINE is a free, open source acid synthesizer, step sequencer and pattern lab. It writes basslines in eight styles, from classic acid house to braindance, lets you twist them live, and sends them to your DAW or to a Roland TB-3.', 'ACID ENGINE es un sintetizador acid, secuenciador por pasos y laboratorio de patrones, gratis y de código abierto. Escribe líneas de bajo en ocho estilos, del acid house clásico al braindance, te deja torcerlas en vivo y las manda a tu DAW o a una Roland TB-3.')}</p>
<div class="grid3">
<div class="card"><h3>{T('WHAT YOU NEED', 'QUÉ NECESITAS')}</h3><p>{T('A computer or phone with <b>Chrome</b>, <b>Edge</b>, Firefox or Safari, and headphones. For MIDI and the TB-3: Chrome or Edge.', 'Una computadora o teléfono con <b>Chrome</b>, <b>Edge</b>, Firefox o Safari, y audífonos. Para MIDI y la TB-3: Chrome o Edge.')}</p></div>
<div class="card v"><h3>{T('WHERE', 'DÓNDE')}</h3><p><a href="{APP}">drophlex.github.io/DFD-ACID-ENGINE/web</a><br>{T('Or the <b>offline file</b> from the website: one HTML that runs from your disk.', 'O el <b>archivo sin internet</b> del sitio: un HTML que corre desde tu disco.')}</p></div>
<div class="card o"><h3>{T('LANGUAGE', 'IDIOMA')}</h3><p>{T('Everything starts in English. <b>ES</b> (top right) switches to Spanish and is remembered.', 'Todo empieza en inglés. <b>ES</b> (arriba a la derecha) lo pasa a español y se recuerda.')}</p></div></div>
<h2>{T('Contents', 'Contenido')}</h2><ol class="toc">{''.join(f'<li><span>{t}</span><i>{i + 3 + OFF:02d}</i></li>' for i, t in enumerate(toc))}</ol>
<p class="small">{T('This manual covers the <b>web</b> version 1.0. The VST3 / AU plugin and the FM-1 firmware get their own manuals when they are released.', 'Este manual cubre la versión <b>web</b> 1.0. El plugin VST3 / AU y el firmware del FM-1 tendrán sus propios manuales cuando salgan.')}</p>''')

    # ------------------------------------------------------------ quick start
    page(T('Start in five minutes', 'Empieza en cinco minutos'), f'''{head('01', T('QUICK START', 'INICIO RÁPIDO'), T('Start in <em>five minutes.</em>', 'Empieza en <em>cinco minutos.</em>'))}
{img('top', T('<b>fig. 1</b> · the machine running. The orbit glows with the level, the line is the live waveform.', '<b>fig. 1</b> · la máquina sonando. La órbita brilla con el nivel, la línea es la onda en vivo.'))}
<ol class="steps">
<li><b>{T('Wake it up.', 'Despiértala.')}</b> {T(f'Open the app and press {k("▶ START THE MACHINE")} (or {k("Space")}). Browsers need one click before they make sound.', f'Abre la app y presiona {k("▶ ENCENDER LA MÁQUINA")} (o {k("Espacio")}). Los navegadores necesitan un clic antes de sonar.')}</li>
<li><b>{T('Pick a style.', 'Elige un estilo.')}</b> {T(f'In {k("01 / GENERATOR")} choose a {k("STYLE")}, a {k("KEY")} and a {k("SCALE")}, then {k("↗ GENERATE NEW PATTERN")}. Press it until you hear the line you want: each press is a new seed.', f'En {k("01 / GENERADOR")} elige un {k("ESTILO")}, un {k("TONO")} y una {k("ESCALA")}, y luego {k("↗ GENERAR PATRÓN NUEVO")}. Presiónalo hasta oír la línea que quieres: cada vez es una semilla nueva.')}</li>
<li><b>{T('Make it squelch.', 'Hazla chillar.')}</b> {T(f'In {k("03 / SOUND DESIGN")} raise {k("RESONANCE")}, then sweep {k("CUTOFF")} and {k("ENV MOD")}. That is the acid sound. {k("DRIVE")} adds teeth.', f'En {k("03 / DISEÑO DE SONIDO")} sube la {k("RESONANCIA")} y barre el {k("CORTE")} y el {k("ENV MOD")}. Ese es el sonido acid. El {k("DRIVE")} le pone dientes.')}</li>
<li><b>{T('Keep what you like.', 'Quédate con lo que te gusta.')}</b> {T(f'Tick {k("DNA LOCK · NOTES")} and generate again: the notes stay, the rhythm changes. Lock {k("RHYTHM")} instead to keep the groove and change the melody.', f'Marca {k("BLOQUEO DE ADN · NOTAS")} y genera otra vez: las notas se quedan y el ritmo cambia. Bloquea {k("RITMO")} para conservar el groove y cambiar la melodía.')}</li>
<li><b>{T('Take it with you.', 'Llévatela.')}</b> {T(f'Drag {k("⇲ DRAG MIDI")} into Ableton, or press {k("↓ MIDI")} / {k("↓ WAV")}. {k("↓ DOWNLOAD PROJECT .JSON")} keeps everything to open later.', f'Arrastra {k("⇲ ARRASTRA MIDI")} a Ableton, o presiona {k("↓ MIDI")} / {k("↓ WAV")}. {k("↓ DESCARGAR PROYECTO .JSON")} guarda todo para abrirlo después.')}</li></ol>
{tip(T(f'Two layers: {k("EASY")} shows what you need to make music; {k("EXPERT")} (top right) opens probability, conditions, the step inspector and every parameter.', f'Dos capas: {k("FÁCIL")} muestra lo necesario para hacer música; {k("EXPERTO")} (arriba a la derecha) abre probabilidad, condiciones, el inspector de paso y todos los parámetros.'))}''')

    # ------------------------------------------------------------ the screen
    page(T('The screen', 'La pantalla'), f'''{head('02', T('THE SCREEN', 'LA PANTALLA'), T('Eight panels, <em>one machine.</em>', 'Ocho paneles, <em>una máquina.</em>'))}
{img('easy', T('<b>fig. 2</b> · the EASY layer: generator, step matrix with sound design, and the archive.', '<b>fig. 2</b> · la capa FÁCIL: generador, matriz de pasos con diseño de sonido, y el archivo.'))}
{table([T('Panel', 'Panel'), T('What it does', 'Qué hace'), T('Page', 'Página')], [
 ['01 / ' + T('GENERATOR', 'GENERADOR'), T('writes patterns in 8 styles, mutates them, locks the genes you like', 'escribe patrones en 8 estilos, los muta, bloquea los genes que te gustan'), '06'],
 ['02 / ' + T('STEP MATRIX', 'MATRIZ DE PASOS'), T('the pattern: notes, accents, slides, ties, ratchets, one cell per 16th', 'el patrón: notas, acentos, slides, ligaduras, ratchets, una celda por semicorchea'), '07'],
 ['03 / ' + T('SOUND DESIGN', 'DISEÑO DE SONIDO'), T('the acid voice: oscillator, filter, envelope, accent, drive', 'la voz acid: oscilador, filtro, envolvente, acento, drive'), '09'],
 ['04 / ' + T('MACHINE MEMORY', 'MEMORIA'), T('factory patterns, your saved patterns, project files, live readouts', 'patrones de fábrica, tus patrones guardados, archivos de proyecto, lecturas en vivo'), '13'],
 ['☠ DIRTY LAB', T('filthy electro house in one button: riff families, dirty sounds, six macros', 'electro house sucio en un botón: familias de riffs, sonidos sucios, seis macros'), '11'],
 ['06 / ' + T('FX RACK', 'RACK DE EFECTOS'), T('eight effects in a chain, all in time with the tempo', 'ocho efectos en cadena, todos al tempo'), '10'],
 ['07 / ' + T('PERFORM', 'EN VIVO'), T('XY pad, hold pads, transpose and keys for playing live', 'pad XY, pads, transposición y teclas para tocar en vivo'), '12'],
 ['008 / TB-3 TWIN LINK', T('the bridge to a Roland TB-3 over USB MIDI', 'el puente a una Roland TB-3 por USB MIDI'), '15']], 'tight')}
<p class="small">{T('The <b>LINK MONITOR</b> line at the bottom of the TB-3 panel tells you what just happened: every export, connection and error is written there.', 'La línea <b>MONITOR DEL LINK</b> al fondo del panel TB-3 te dice qué acaba de pasar: cada exportación, conexión y error se escribe ahí.')}</p>''')

    # ------------------------------------------------------------ toolbar
    page(T('Toolbar and transport', 'Barra y transporte'), f'''{head('03', T('TOOLBAR', 'BARRA'), T('Transport, tempo <em>and exits.</em>', 'Transporte, tempo <em>y salidas.</em>'))}
{img('toolbar', T('<b>fig. 3</b> · the toolbar (EASY layer).', '<b>fig. 3</b> · la barra (capa FÁCIL).'))}
{table([T('Control', 'Control'), T('What it does', 'Qué hace')], [
 [k('▶ PLAY') + ' ' + k('■ STOP'), T(f'start and stop. {k("Space")} does the same from anywhere on the page.', f'arranca y detiene. {k("Espacio")} hace lo mismo desde cualquier parte de la página.')],
 [k(T('TEMPO', 'TEMPO')), T('30 to 300 BPM. Type a number and press Enter.', '30 a 300 BPM. Escribe un número y presiona Enter.')],
 [k(T('STEPS', 'PASOS')), T('pattern length: 8, 16, 24, 32, 48, 64, 128 or 256 sixteenths. Longer patterns repeat the existing steps.', 'largo del patrón: 8, 16, 24, 32, 48, 64, 128 o 256 semicorcheas. Los patrones más largos repiten los pasos existentes.')],
 [k('SWING'), T('delays every second 16th, up to 75 %. 15–30 % is a classic house shuffle.', 'retrasa cada segunda semicorchea, hasta 75 %. 15–30 % es el shuffle clásico de house.')],
 [k(T('EDIT', 'EDITAR')) + ' A / B', T('there are always two patterns. This chooses which one you see and edit.', 'siempre hay dos patrones. Esto elige cuál ves y editas.')],
 [k('A ⇄ B MORPH'), T('blends A into B step by step while it plays: at 50 % about half of the steps come from B. Great for slow builds.', 'mezcla A con B paso a paso mientras suena: al 50 % cerca de la mitad de los pasos vienen de B. Ideal para subidas lentas.')],
 [k(T('QUALITY', 'CALIDAD')) + ' <span class="x">EXPERT</span>', T('oversampling: ECO 1× (phones), HQ 2× (default), ULTRA 4× (cleanest highs, more CPU).', 'sobremuestreo: ECO 1× (teléfonos), HQ 2× (por defecto), ULTRA 4× (agudos más limpios, más CPU).')],
 [k('↓ MIDI') + ' ' + k(T('⇲ DRAG MIDI', '⇲ ARRASTRA MIDI')), T('a .mid of exactly what you hear (page 13). Drag the dashed button straight into a DAW.', 'un .mid de exactamente lo que oyes (página 13). Arrastra el botón punteado directo a un DAW.')],
 [k('↓ WAV') + ' ' + k('↓ STEMS') + ' <span class="x">EXPERT</span>', T('render audio at 48 kHz / 24-bit, faster than real time.', 'renderiza audio a 48 kHz / 24 bits, más rápido que en tiempo real.')],
 [k(T('PANIC', 'PÁNICO')), T('silences every note, in the browser and on the TB-3.', 'calla todas las notas, en el navegador y en la TB-3.')]])}
{tip(T(f'Undo and redo any pattern edit with {k("Ctrl/⌘ + Z")} and {k("Ctrl/⌘ + Shift + Z")}.', f'Deshaz y rehaz cualquier cambio del patrón con {k("Ctrl/⌘ + Z")} y {k("Ctrl/⌘ + Shift + Z")}.'))}''')

    # ------------------------------------------------------------ generator
    styles = [('ACID HOUSE / CLASSIC', '120–128', T('the root note pumping with octave jumps, minor third and seventh; accents on the beat, slides into the high notes', 'la tónica bombeando con saltos de octava, tercera menor y séptima; acentos en el tiempo, slides hacia las notas altas')),
              ('ACID TECHNO / 3-3-2', '130–145', T('sixteen notes per bar, accents in the 3-3-2 pattern, a four-step motif that hypnotizes', 'dieciséis notas por compás, acentos en patrón 3-3-2, un motivo de cuatro pasos que hipnotiza')),
              ('FILTER HOUSE / FRENCH', '118–126', T('long notes on the off-beats, octaves and sevenths, ties: made for sweeping the cutoff slowly', 'notas largas en los contratiempos, octavas y séptimas, ligaduras: hecho para barrer el corte despacio')),
              ('BIG BEAT / BREAKS', '100–130', T('a riff with blues notes (4th, flat 5th), strong syncopated accents, a full bar phrase', 'un riff con notas de blues (4ª, 5ª bemol), acentos sincopados fuertes, una frase de un compás')),
              ('RAVE / HARDCORE', '140–160', T('minor chords broken into a line, octaves up, lots of slides', 'acordes menores rotos en una línea, octavas arriba, muchos slides')),
              ('BRAINDANCE / DRILL', '150–180', T('fast ratchets, odd intervals, probability: always anchored on the root so it stays musical', 'ratchets rápidos, intervalos raros, probabilidad: siempre anclado en la tónica para que siga musical')),
              ('ELECTRO / PUNK', '125–135', T('a driving root and octave pulse in short, hard notes', 'un pulso de tónica y octava que empuja, en notas cortas y duras')),
              ('DEEP / HYPNOTIC', '118–124', T('few notes, long gates and ties: space for the filter and the delay', 'pocas notas, gates largos y ligaduras: espacio para el filtro y el delay')),
              ('ELECTRO HOUSE / DIRTY', '126–130', T('octave riffs on distorted saw walls, offbeat accents, short notes, pump', 'riffs en octavas sobre muros de sierras distorsionadas, acentos a contratiempo, notas cortas, bombeo')),
              ('BLOGHOUSE / FIDGET', '126–132', T('choppy syncopation, pitch zaps, wider leaps', 'síncopa entrecortada, zaps de tono, saltos más amplios'))]
    page(T('The generator', 'El generador'), f'''{head('04', T('GENERATOR', 'GENERADOR'), T('Ten styles that <em>groove.</em>', 'Diez estilos con <em>groove.</em>'))}
<div class="two g"><div class="tall">{img('gen_expert', T('<b>fig. 4</b> · EXPERT layer.', '<b>fig. 4</b> · capa EXPERTO.'))}</div><div>
<p class="small">{T('Picking a style also loads its own sound.', 'Elegir un estilo también carga su propio sonido.')}</p><p>{T('Every pattern is built like a real riff: a short motif on the root of your key, repeated with small changes, with a turnaround at the end of the bar. The generator writes <b>16 versions</b> and keeps the most usable one (on the root, singable leaps, a groove that repeats). The same seed always gives the same pattern.', 'Cada patrón se construye como un riff real: un motivo corto sobre la tónica de tu tono, repetido con pequeños cambios y con una vuelta al final del compás. El generador escribe <b>16 versiones</b> y se queda con la más usable (en la tónica, saltos cantables, un groove que se repite). La misma semilla siempre da el mismo patrón.')}</p>
{table([T('Style', 'Estilo'), 'BPM', T('Character', 'Carácter')], [[f'<b>{a}</b>', b, c] for a, b, c in styles], 'tight small')}</div></div>
{table([T('Control', 'Control'), T('What it does', 'Qué hace')], [
 [k(T('KEY', 'TONO')) + ' ' + k(T('SCALE', 'ESCALA')), T('the root note and the notes allowed. MINOR, PHRYGIAN and DORIAN are the acid classics; CHROMATIC lets the style use any note.', 'la nota base y las notas permitidas. MENOR, FRIGIO y DÓRICO son los clásicos del acid; CROMÁTICA deja al estilo usar cualquier nota.')],
 [k(T('DENSITY', 'DENSIDAD')) + ' ' + k(T('CHAOS', 'CAOS')), T('how many notes, and how far the line wanders from its motif', 'cuántas notas, y qué tanto se aleja la línea de su motivo')],
 [k(T('MUTATION', 'MUTACIÓN')) + ' + ' + k(T('↻ MUTATE', '↻ MUTAR')), T('changes the current pattern by that amount, staying in the style', 'cambia el patrón actual en esa cantidad, sin salir del estilo')],
 [k(T('DNA LOCK', 'BLOQUEO DE ADN')), T('NOTES · RHYTHM · ACCENTS · SLIDES: the locked genes survive Generate, Mutate and Shuffle', 'NOTAS · RITMO · ACENTOS · SLIDES: los genes bloqueados sobreviven a Generar, Mutar y Revolver')],
 [k(T('PROBABILITY', 'PROBABILIDAD')) + ' ' + k(T('CONDITIONS', 'CONDICIONES')) + ' <span class="x">EXPERT</span>', T('add steps that play only sometimes, or only on certain loops (page 08)', 'agregan pasos que suenan solo a veces, o solo en ciertas vueltas (página 08)')],
 [k(T('POLYRHYTHM', 'POLIRRITMO')) + ' ' + k('SEED') + ' <span class="x">EXPERT</span>', T('accents every 3, 5 or 7 steps across the bar; type a seed to get a pattern back, the dice button for a new one', 'acentos cada 3, 5 o 7 pasos a lo largo del compás; escribe una semilla para recuperar un patrón, el botón del dado para una nueva')]], 'tight')}''')

    # ------------------------------------------------------------ step matrix
    page(T('The step matrix', 'La matriz de pasos'), f'''{head('05', T('STEP MATRIX', 'MATRIZ DE PASOS'), T('One cell, <em>one sixteenth.</em>', 'Una celda, <em>una semicorchea.</em>'))}
{img('grid', T('<b>fig. 5</b> · each cell: step number (click to inspect), the note, the gate bar, and the four switches A S T R.', '<b>fig. 5</b> · cada celda: número de paso (clic para inspeccionar), la nota, la barra de gate y los cuatro interruptores A S T R.'))}
<div class="two"><div>{table([T('Do this', 'Haz esto'), T('To', 'Para')], [
 [T('click the note', 'clic en la nota'), T('turn the step on / off', 'encender / apagar el paso')],
 [T('right-click', 'clic derecho'), T('one semitone up', 'un semitono arriba')],
 [T('Shift + click', 'Shift + clic'), T('one semitone down', 'un semitono abajo')],
 [T('mouse wheel', 'rueda del mouse'), T('pitch up / down', 'tono arriba / abajo')],
 [T('click the number', 'clic en el número'), T('select it for the inspector', 'seleccionarlo para el inspector')],
 [k('◀') + ' ' + k('▶'), T('rotate the whole pattern one step', 'rotar todo el patrón un paso')],
 [k(T('CLEAR', 'BORRAR')) + ' ' + k(T('SHUFFLE', 'REVOLVER')), T('empty it / mutate it hard', 'vaciarlo / mutarlo fuerte')]], 'tight')}</div><div>
{table([T('Switch', 'Interruptor'), T('Means', 'Significa')], [
 ['<span class="sw o">A</span> ' + T('ACCENT', 'ACENTO'), T('louder and brighter. Accents close together build up and get wilder, like the classic machines.', 'más fuerte y más brillante. Los acentos seguidos se acumulan y se ponen más salvajes, como en las máquinas clásicas.')],
 ['<span class="sw v">S</span> SLIDE', T('glides from the previous note into this one, without a new attack', 'se desliza desde la nota anterior hasta esta, sin ataque nuevo')],
 ['<span class="sw b">T</span> ' + T('TIE', 'LIGADURA'), T('holds the previous note through this step (same pitch)', 'sostiene la nota anterior durante este paso (mismo tono)')],
 ['<span class="sw a">R</span> RATCHET', T('repeats the note 2, 3 or 4 times inside the step (up to 8 in the inspector)', 'repite la nota 2, 3 o 4 veces dentro del paso (hasta 8 en el inspector)')]], 'tight')}
<p class="small">{T('Badges in the corner show ratchets (×3), probability (50 %) and conditions (1:2). The violet border marks every 8 steps; the glowing cell is the playhead.', 'Las etiquetas de la esquina muestran ratchets (×3), probabilidad (50 %) y condiciones (1:2). El borde violeta marca cada 8 pasos; la celda que brilla es el cabezal.')}</p></div></div>
{tip(T('Everything can be edited while it plays. Patterns A and B have the same length; switch between them with EDIT A / B.', 'Todo se puede editar mientras suena. Los patrones A y B tienen el mismo largo; cambia entre ellos con EDITAR A / B.'))}''')

    # ------------------------------------------------------------ inspector + conditions
    page(T('Step inspector and conditions', 'Inspector de paso y condiciones'), f'''{head('06', T('STEP INSPECTOR', 'INSPECTOR DE PASO') + ' · EXPERT', T('Every detail <em>of a step.</em>', 'Cada detalle <em>de un paso.</em>'))}
{img('inspector', T('<b>fig. 6</b> · click a step number, then shape it here.', '<b>fig. 6</b> · haz clic en el número de un paso y dale forma aquí.'))}
{table([T('Field', 'Campo'), T('Range', 'Rango'), T('What it does', 'Qué hace')], [
 ['NOTE', 'C1 – C7', T('the pitch (turns the step on)', 'el tono (enciende el paso)')],
 ['GATE', '5 – 100 %', T('how long the note lasts inside the step. 100 % touches the next one', 'cuánto dura la nota dentro del paso. 100 % toca la siguiente')],
 ['VELOCITY', '1 – 127', T('how hard; also sent as MIDI velocity. Accents always send 127', 'qué tan fuerte; también se manda como velocidad MIDI. Los acentos siempre mandan 127')],
 [T('PROBABILITY', 'PROBABILIDAD'), '0 – 100 %', T('chance that the step plays each time. The dice are fixed by the seed, so exports match what you hear', 'probabilidad de que el paso suene cada vez. Los dados los fija la semilla, así que lo exportado coincide con lo que oyes')],
 ['MICRO', '± 50 %', T('pushes the step early or late, by up to half a step', 'adelanta o atrasa el paso, hasta medio paso')],
 ['RATCHET', '1 – 8', T('hits inside the step', 'golpes dentro del paso')]], 'tight')}
<h2>{T('Conditions', 'Condiciones')}</h2>
<p>{T('A condition decides <b>on which loops</b> a step plays. They turn a one-bar loop into a phrase that evolves.', 'Una condición decide <b>en qué vueltas</b> suena un paso. Convierten un loop de un compás en una frase que evoluciona.')}</p>
{table([T('Condition', 'Condición'), T('Plays', 'Suena')], [
 ['ALWAYS', T('every time (default)', 'siempre (por defecto)')], ['FILL', T(f'only while {k("FILL")} is held (pad or key {k("F")})', f'solo mientras mantienes {k("FILL")} (pad o tecla {k("F")})')], ['!FILL', T('only when FILL is not held', 'solo cuando FILL no está presionado')],
 ['1ST', T('only on the first loop after PLAY', 'solo en la primera vuelta después de PLAY')], ['!1ST', T('every loop except the first', 'todas las vueltas menos la primera')],
 ['A:B', T('on loop A of every B: 1:2 = odd loops, 2:2 = even loops, 4:4 = the last of every four, 1:8…', 'en la vuelta A de cada B: 1:2 = vueltas impares, 2:2 = pares, 4:4 = la última de cada cuatro, 1:8…')]], 'tight')}''')

    # ------------------------------------------------------------ sound design
    page(T('Sound design', 'Diseño de sonido'), f'''{head('07', T('SOUND DESIGN', 'DISEÑO DE SONIDO'), T('The voice: <em>squelch on demand.</em>', 'La voz: <em>squelch a la orden.</em>'))}
{img('sound', T('<b>fig. 7</b> · EXPERT shows the full voice. Double-click any slider to reset it.', '<b>fig. 7</b> · EXPERTO muestra la voz completa. Doble clic en cualquier slider lo regresa a su valor.'), '50%')}
{table([T('Parameter', 'Parámetro'), T('What it does', 'Qué hace')], [
 [T('VOICE', 'VOZ'), T('ACID: one oscillator, the classic. DIRTY: a wall of seven detuned saws (DETUNE widens it). FIDGET: two hard-detuned oscillators plus one an octave up', 'ACID: un oscilador, el clásico. DIRTY: un muro de siete sierras desafinadas (DESAFINACIÓN lo abre). FIDGET: dos osciladores muy desafinados más uno una octava arriba')],
 [T('SAW ⇄ SQUARE', 'ONDA'), T('blends the two classic waves; PULSE WIDTH shapes the square', 'mezcla las dos ondas clásicas; ANCHO DE PULSO da forma a la cuadrada')],
 [T('FILTH · PITCH DROP', 'SUCIEDAD · CAÍDA DE TONO'), T('a wavefolder for grind; each note starting up to two octaves high and falling (DROP TIME sets how fast): the electro zap', 'un plegador de onda para ensuciar; cada nota empieza hasta dos octavas arriba y cae (TIEMPO DE CAÍDA marca qué tan rápido): el zap del electro')],
 [T('CUTOFF · RESONANCE', 'CORTE · RESONANCIA'), T('a 4-pole resonant low-pass. Resonance near the top whistles and self-oscillates; the bass is compensated so it does not get thin', 'un pasa-bajos resonante de 4 polos. La resonancia arriba silba y auto-oscila; los graves se compensan para que no se adelgace')],
 ['ENV MOD · ' + T('DECAY', 'CAÍDA'), T('how far and how long the envelope opens the filter on each note', 'qué tanto y por cuánto tiempo la envolvente abre el filtro en cada nota')],
 [T('ACCENT', 'ACENTO'), T('how much accented steps jump in volume and filter', 'cuánto saltan en volumen y filtro los pasos con acento')],
 ['SUB · ' + T('NOISE', 'RUIDO'), T('a sine one octave down for weight; noise for grit', 'una senoidal una octava abajo para dar peso; ruido para dar textura')],
 ['DRIVE · OVERDRIVE · ' + T('TONE', 'TONO'), T('drive before the filter (fat), overdrive after it (raw); tone tilts the result darker or brighter', 'drive antes del filtro (gordo), overdrive después (crudo); tono inclina el resultado a oscuro o brillante')],
 [T('SLIDE TIME', 'TIEMPO DE SLIDE') + ' · ' + T('AMP DECAY', 'CAÍDA DE AMP'), T('glide speed between slid notes; how the volume fades while a long note is held', 'velocidad del glide entre notas con slide; cómo baja el volumen mientras se sostiene una nota larga')],
 [T('VOLUME', 'VOLUMEN'), T('output level; a soft limiter never lets it clip', 'nivel de salida; un limitador suave nunca deja que sature')]], 'tight')}''')

    # ------------------------------------------------------------ FX
    page(T('FX rack', 'Rack de efectos'), f'''{head('08', T('FX RACK', 'RACK DE EFECTOS'), T('Nine effects, <em>in time.</em>', 'Nueve efectos, <em>a tiempo.</em>'))}
{img('fx', T('<b>fig. 8</b> · click a title to switch the effect on (lime) or off. EASY shows the main knob of each; EXPERT shows all.', '<b>fig. 8</b> · haz clic en un título para encender (verde) o apagar el efecto. FÁCIL muestra la perilla principal de cada uno; EXPERTO las muestra todas.'), '64%')}
{table([T('Effect', 'Efecto'), T('Controls', 'Controles'), T('Use it for', 'Úsalo para')], [
 ['CRUSH', T('mix · bits · rate', 'mezcla · bits · frecuencia'), T('lo-fi grit, broken digital edges', 'suciedad lo-fi, bordes digitales rotos')],
 ['RING MOD', T('mix · frequency', 'mezcla · frecuencia'), T('metallic, bell-like tones', 'tonos metálicos, de campana')],
 ['STUTTER', T('length 1/4 · 1/8 · 1/16 · 1/32', 'largo 1/4 · 1/8 · 1/16 · 1/32'), T('freezes and repeats a slice in time. Best held from the STUTTER pad', 'congela y repite un pedazo a tiempo. Mejor desde el pad STUTTER')],
 ['DYN FILTER', T('depth · cycle (steps) · resonance', 'profundidad · ciclo (pasos) · resonancia'), T('a second filter that sweeps in time, from 1 to 64 steps', 'un segundo filtro que barre a tiempo, de 1 a 64 pasos')],
 ['CHORUS · FLANGER · PHASER', T('mix · type · rate · depth · feedback', 'mezcla · tipo · velocidad · profundidad · realimentación'), T('width, jet sweeps, swirl', 'amplitud, barridos tipo jet, remolinos')],
 ['SIDECHAIN PUMP', T('depth', 'profundidad'), T('ducks on every beat and breathes back, locked to the tempo: the electro house pump', 'baja en cada tiempo y vuelve a respirar, al tempo: el bombeo del electro house')],
 ['COMPRESSOR', T('threshold · ratio · makeup', 'umbral · ratio · ganancia'), T('on by default: keeps accents punchy and the level even', 'encendido por defecto: mantiene los acentos con golpe y el nivel parejo')],
 ['DELAY', T('mix · time (1/16 … 1/4D) · feedback · tone · ping-pong', 'mezcla · tiempo (1/16 … 1/4D) · realimentación · tono · ping-pong'), T('dub echoes. 3/16 is the classic acid delay', 'ecos dub. 3/16 es el delay acid clásico')],
 ['REVERB', T('mix · size · damping', 'mezcla · tamaño · amortiguación'), T('space; keep the mix low on basslines', 'espacio; deja la mezcla baja en líneas de bajo')]], 'tight')}
<p class="small">{T('Order of the chain: pump → crush → ring → stutter → dynamic filter → chorus / flanger / phaser → compressor → delay → reverb.', 'Orden de la cadena: pump → crush → ring → stutter → filtro dinámico → chorus / flanger / phaser → compresor → delay → reverb.')}</p>''')

    # ------------------------------------------------------------ DIRTY LAB
    page('DIRTY LAB', f'''{head('☠', T('SPECIAL SECTION', 'SECCIÓN ESPECIAL'), T('DIRTY <em class="o">LAB.</em>', 'DIRTY <em class="o">LAB.</em>'))}
<p class="lead">{T('Filthy electro house in one button: distorted saw walls, octave riffs, zaps and the pump. Press <b>☠ DIRTY GENERATE</b> and it picks a riff family and a dirty sound, writes pattern A and a variation in B, and starts playing.', 'Electro house sucio en un botón: muros de sierras distorsionadas, riffs en octavas, zaps y el bombeo. Presiona <b>☠ GENERAR DIRTY</b> y elige una familia de riffs y un sonido sucio, escribe el patrón A y una variación en B, y empieza a tocar.')}</p>
{img('lab', T('<b>fig. 9</b> · the DIRTY LAB. The orange button in the toolbar jumps here.', '<b>fig. 9</b> · el DIRTY LAB. El botón naranja de la barra salta aquí.'))}
<div class="two"><div>{table([T('Riff family', 'Familia'), T('What it does', 'Qué hace')], [
 ['OCTAVE PUMP', T('root and octave on the 16ths, accents on the offbeat', 'tónica y octava en semicorcheas, acentos a contratiempo')],
 ['ZAP BOUNCE', T('sparse, bouncing hits made for the pitch drop', 'golpes espaciados que rebotan, hechos para la caída de tono')],
 ['GRIND 16THS', T('a relentless 16th grind on the root', 'una molienda de semicorcheas sobre la tónica')],
 ['CALL & RESPONSE', T('a low phrase answered high, one bar', 'una frase grave contestada arriba, un compás')],
 ['STAB GATE', T('short stabs on the offbeats, room for the drums', 'golpes cortos a contratiempo, espacio para la batería')],
 ['DROP RIDE', T('busy, rolling, slides and ratchets for the drop', 'cargado, rodando, slides y ratchets para el drop')]], 'tight small')}</div><div>
{table(['MACRO', T('Moves', 'Mueve')], [
 [T('FILTH', 'SUCIEDAD'), T('fold, drive, overdrive, crush', 'plegado, drive, overdrive, crush')], [T('PUMP', 'BOMBEO'), T('the sidechain duck', 'el ducking de sidechain')],
 ['ZAP', T('pitch drop and its time', 'caída de tono y su tiempo')], [T('WIDTH', 'ANCHO'), T('the detune of the saw wall', 'la desafinación del muro de sierras')],
 [T('SCREAM', 'GRITO'), T('resonance and env mod', 'resonancia y env mod')], [T('BITE', 'MORDIDA'), T('cutoff', 'corte')]], 'tight small')}
<p class="small">{T('9 dirty sounds: ROTTEN SAW · TEUTON GRIND · ZAP CANNON · OCTAVE MONSTER · 8-BIT BRUISER · SQUEAL PIG · BASS SLAB · GUTTER STAB · RAVE SIREN.', '9 sonidos sucios: ROTTEN SAW · TEUTON GRIND · ZAP CANNON · OCTAVE MONSTER · 8-BIT BRUISER · SQUEAL PIG · BASS SLAB · GUTTER STAB · RAVE SIREN.')}</p></div></div>
{tip(T('Ride A ⇄ B MORPH on the XY pad: the lab always writes B as a variation of A.', 'Recorre el MORPH A ⇄ B en el pad XY: el lab siempre escribe B como una variación de A.'))}''', 'dirtyp')

    # ------------------------------------------------------------ perform
    page(T('Perform live', 'Tocar en vivo'), f'''{head('09', T('PERFORM', 'EN VIVO'), T('Hands on <em>the machine.</em>', 'Manos sobre <em>la máquina.</em>'))}
{img('perform', T('<b>fig. 9</b> · XY pad, hold pads, transpose and keys. Works with a mouse, a trackpad or fingers.', '<b>fig. 9</b> · pad XY, pads, transposición y teclas. Funciona con mouse, trackpad o dedos.'))}
{table([T('Control', 'Control'), T('What it does', 'Qué hace')], [
 ['XY', T('left ↔ right = A ⇄ B morph, down ↕ up = cutoff. One finger, two moves.', 'izquierda ↔ derecha = morph A ⇄ B, abajo ↕ arriba = corte. Un dedo, dos movimientos.')],
 [k('FILL'), T(f'hold: plays the FILL steps and skips the !FILL steps. Key {k("F")}.', f'mantener: toca los pasos FILL y salta los !FILL. Tecla {k("F")}.')],
 [k('STUTTER'), T('hold: repeats the current slice; let go to drop back in', 'mantener: repite el pedazo actual; suelta para regresar')],
 [k('MUTE'), T('toggle: silences the line (and the TB-3) without stopping the clock', 'alternar: calla la línea (y la TB-3) sin detener el reloj')],
 [k('SQUELCH'), T('hold: filter open, resonance and env mod to the max. Let go: back to your sound', 'mantener: filtro abierto, resonancia y env mod al máximo. Suelta: regresa a tu sonido')],
 [k('−12') + ' ' + k('−5') + ' ' + k('0') + ' ' + k('+7'), T('transpose the whole pattern live (also the MIDI and the TB-3)', 'transpone todo el patrón en vivo (también el MIDI y la TB-3)')],
 [T('keys', 'teclas'), T(f'one octave from the root of your key. Computer keys {k("Z S X D C V G B H N J M ,")}', f'una octava desde la tónica de tu tono. Teclas de la computadora {k("Z S X D C V G B H N J M ,")}')]], 'tight')}
{tip(T('A live arc in four moves: start at morph 0 with the filter low, raise the cutoff on the XY, slide right towards B, hold SQUELCH on the last bar before the drop.', 'Un arco en vivo en cuatro movimientos: empieza en morph 0 con el filtro bajo, sube el corte en el XY, deslízate a la derecha hacia B, y mantén SQUELCH en el último compás antes del drop.'))}''')

    # ------------------------------------------------------------ archive + exports
    page(T('Archive and exports', 'Archivo y exportación'), f'''{head('10', T('ARCHIVE · EXPORT', 'ARCHIVO · EXPORTAR'), T('Keep it. <em>Take it out.</em>', 'Guárdalo. <em>Sácalo.</em>'))}
<div class="two"><div>{img('archive', '', '100%', '4.1in')}</div><div>
{table([T('Button', 'Botón'), T('What it does', 'Qué hace')], [
 [T('FACTORY DNA', 'ADN DE FÁBRICA'), T('ten original DFD / POLARIS patterns with their own sound and tempo', 'diez patrones originales DFD / POLARIS con su propio sonido y tempo')],
 [T('SAVE IN THIS BROWSER', 'GUARDAR EN ESTE NAVEGADOR'), T('stores pattern A and B, sound, FX and generator settings under the name you type. Only on this browser and computer', 'guarda los patrones A y B, el sonido, los efectos y el generador con el nombre que escribas. Solo en este navegador y computadora')],
 [T('↓ PROJECT .JSON', '↓ PROYECTO .JSON'), T('the same as a file: back it up, share it, open it anywhere with ↑ IMPORT', 'lo mismo en un archivo: respáldalo, compártelo, ábrelo donde sea con ↑ IMPORTAR')],
 ['PATTERN DNA', T('a fingerprint of the pattern, also used in file names', 'una huella del patrón, también se usa en los nombres de archivo')]], 'tight small')}</div></div>
<h2>{T('What you hear is what you get', 'Lo que oyes es lo que obtienes')}</h2>
<p>{T('Exports are rendered by the same engine, faster than real time, so probability, conditions, ratchets, swing, micro-timing and the A ⇄ B morph come out exactly as they play.', 'Las exportaciones las renderiza el mismo motor, más rápido que en tiempo real, así que la probabilidad, las condiciones, los ratchets, el swing, el micro-timing y el morph A ⇄ B salen exactamente como suenan.')}</p>
{table([T('Export', 'Exportar'), T('You get', 'Obtienes')], [
 ['↓ MIDI', T('a .mid (480 PPQ) of about 64 steps, with tempo. Slides are legato notes plus CC 102; accents are velocity 127 plus CC 103', 'un .mid (480 PPQ) de unos 64 pasos, con tempo. Los slides son notas legato más CC 102; los acentos son velocidad 127 más CC 103')],
 [T('⇲ DRAG MIDI', '⇲ ARRASTRA MIDI'), T('the same file, dragged straight onto a DAW track or the desktop (Chrome / Edge)', 'el mismo archivo, arrastrado directo a una pista del DAW o al escritorio (Chrome / Edge)')],
 ['↓ WAV', T('about 8 bars plus the effect tail, 48 kHz / 24-bit stereo', 'unos 8 compases más la cola de los efectos, 48 kHz / 24 bits estéreo')],
 ['↓ STEMS', T('three aligned WAVs: SYNTH DRY, FX (wet only) and FULL, for mixing in your DAW', 'tres WAV alineados: SYNTH DRY (seco), FX (solo efecto) y FULL (completo), para mezclar en tu DAW')]], 'tight')}''')

    # ------------------------------------------------------------ ableton
    page(T('With Ableton Live', 'Con Ableton Live'), f'''{head('11', 'ABLETON LIVE', T('From the browser <em>to the set.</em>', 'Del navegador <em>al set.</em>'))}
{img('toolbar', T('<b>fig. 11</b> · ⇲ DRAG MIDI is the dashed violet button.', '<b>fig. 11</b> · ⇲ ARRASTRA MIDI es el botón violeta punteado.'))}
<h2>{T('A · Drag the line in', 'A · Arrastra la línea')}</h2>
<ol class="steps"><li>{T('Set Live and ACID ENGINE to the same tempo.', 'Pon Live y ACID ENGINE al mismo tempo.')}</li>
<li>{T(f'Drag {k("⇲ DRAG MIDI")} onto an empty MIDI track: it becomes a clip. (Safari / Firefox: press {k("↓ MIDI")} and drag the file from your Downloads.)', f'Arrastra {k("⇲ ARRASTRA MIDI")} a una pista MIDI vacía: se vuelve un clip. (Safari / Firefox: presiona {k("↓ MIDI")} y arrastra el archivo desde tus Descargas.)')}</li>
<li>{T('Put any bass synth on the track, or route it to your TB-3 (below). Accents are velocity 127: map velocity to filter in your synth for the full effect.', 'Pon cualquier sinte de bajo en la pista, o mándala a tu TB-3 (abajo). Los acentos son velocidad 127: asigna la velocidad al filtro de tu sinte para el efecto completo.')}</li></ol>
<h2>{T('B · Bring the audio in', 'B · Trae el audio')}</h2>
<ol class="steps"><li>{T(f'{k("↓ WAV")} for the full sound, or {k("↓ STEMS")} to process the dry synth and the effects on separate tracks.', f'{k("↓ WAV")} para el sonido completo, o {k("↓ STEMS")} para procesar el sinte seco y los efectos en pistas separadas.')}</li>
<li>{T('Drop the WAV into an audio track. It starts exactly on the bar, so Warp it with <b>Beats</b> mode at the tempo you exported.', 'Suelta el WAV en una pista de audio. Empieza justo en el compás, así que ajústalo (Warp) en modo <b>Beats</b> al tempo que exportaste.')}</li></ol>
<h2>{T('C · Play a TB-3 from Live with ACID ENGINE patterns', 'C · Toca una TB-3 desde Live con patrones de ACID ENGINE')}</h2>
<ol class="steps"><li>{T('Connect the TB-3 by USB. In Live: Preferences › Link, Tempo & MIDI › turn on <b>Track</b> for the TB-3 output.', 'Conecta la TB-3 por USB. En Live: Preferencias › Link, Tempo & MIDI › activa <b>Track</b> en la salida de la TB-3.')}</li>
<li>{T('On the MIDI track with the dragged clip: MIDI To → TB-3, channel 1. Live sends the notes, legato slides and CC 102 / 103.', 'En la pista MIDI con el clip arrastrado: MIDI To → TB-3, canal 1. Live manda las notas, los slides legato y los CC 102 / 103.')}</li>
<li>{T('Record the TB-3 sound through your audio interface (or the TB-3 USB audio, if your setup supports it).', 'Graba el sonido de la TB-3 con tu interfaz de audio (o el audio USB de la TB-3, si tu equipo lo permite).')}</li></ol>
{tip(T('Use Live OR the browser to drive the TB-3 at one time, not both: two senders on the same channel double the notes.', 'Usa Live O el navegador para manejar la TB-3, no los dos a la vez: dos fuentes en el mismo canal duplican las notas.'))}''')

    # ------------------------------------------------------------ TB-3
    diag = f'''<svg class="diag" viewBox="0 0 700 150" xmlns="http://www.w3.org/2000/svg"><g font-family="TeX Gyre Heros,Helvetica,Arial" font-weight="700">
<rect x="10" y="30" width="200" height="90" fill="#1b1c22" stroke="#c1ff3a"/><text x="110" y="70" fill="#c1ff3a" font-size="16" text-anchor="middle">ACID ENGINE</text><text x="110" y="94" fill="#9a9ca3" font-size="11" text-anchor="middle">CHROME / EDGE</text>
<rect x="490" y="30" width="200" height="90" fill="#1b1c22" stroke="#a799ff"/><text x="590" y="70" fill="#a799ff" font-size="16" text-anchor="middle">TB-3</text><text x="590" y="94" fill="#9a9ca3" font-size="11" text-anchor="middle">USB · {T('DATA CABLE', 'CABLE DE DATOS')}</text>
<line x1="214" y1="62" x2="486" y2="62" stroke="#c1ff3a" stroke-width="3"/><polygon points="486,62 474,55 474,69" fill="#c1ff3a"/><text x="350" y="52" fill="#c1ff3a" font-size="11" text-anchor="middle">{T('NOTES · SLIDE CC102 · ACCENT CC103 · SYSEX', 'NOTAS · SLIDE CC102 · ACENTO CC103 · SYSEX')}</text>
<line x1="486" y1="94" x2="214" y2="94" stroke="#ff7147" stroke-width="3"/><polygon points="214,94 226,87 226,101" fill="#ff7147"/><text x="350" y="116" fill="#ff7147" font-size="11" text-anchor="middle">{T('NOTES · CC · PATCH BLOCKS', 'NOTAS · CC · BLOQUES DEL PATCH')}</text></g></svg>'''
    page('TB-3 TWIN LINK', f'''{head('12', T('HARDWARE', 'HARDWARE'), T('One pattern, <em>two machines.</em>', 'Un patrón, <em>dos máquinas.</em>'))}
{diag}
<ol class="steps"><li>{T('TB-3 to the computer with a USB <b>data</b> cable (a charge-only cable shows nothing). Open the app in <b>Chrome</b> or <b>Edge</b>.', 'TB-3 a la computadora con un cable USB <b>de datos</b> (un cable solo de carga no muestra nada). Abre la app en <b>Chrome</b> o <b>Edge</b>.')}</li>
<li>{T(f'{k("CONNECT USB MIDI")} → allow MIDI. Allow SysEx too if you want the Sound Lab (read / backup / restore).', f'{k("CONECTAR USB MIDI")} → permite MIDI. Permite también SysEx si quieres el Sound Lab (leer / respaldar / restaurar).')}</li>
<li>{T(f'Choose the {k("TWIN LINK")} mode and the TB-3 MIDI channel (1 by default).', f'Elige el modo {k("TWIN LINK")} y el canal MIDI de la TB-3 (1 por defecto).')}</li></ol>
{table([T('Mode', 'Modo'), T('What happens', 'Qué pasa')], [
 ['OFF · SAFE', T('nothing is sent to the TB-3. Default.', 'no se manda nada a la TB-3. Por defecto.')],
 ['VIRTUAL', T('the TB-3 plays the web synth: its notes trigger the browser voice and its cutoff / resonance knobs (CC 74 / 71) move the browser filter', 'la TB-3 toca el sinte web: sus notas disparan la voz del navegador y sus perillas de corte / resonancia (CC 74 / 71) mueven el filtro del navegador')],
 ['HARDWARE', T('the sequencer plays only the TB-3; the browser stays silent', 'el secuenciador toca solo la TB-3; el navegador se queda callado')],
 ['TWIN', T('both at once, in time: notes are scheduled to land when the browser audio is heard', 'los dos a la vez, a tiempo: las notas se programan para caer cuando se escucha el audio del navegador')]], 'tight')}
<div class="two"><div><h2>SHADOW CAPTURE</h2><p class="small">{T(f'{k("● START RECORDING")}, play a pattern on the TB-3, then {k("■ STOP + QUANTIZE")}: the notes land on the 16ths of the pattern you are editing (strong velocities become accents). Then mutate it and send it back in TWIN.', f'{k("● EMPEZAR A GRABAR")}, toca un patrón en la TB-3 y luego {k("■ DETENER + CUANTIZAR")}: las notas caen en las semicorcheas del patrón que editas (las velocidades fuertes se vuelven acentos). Luego mútalo y devuélvelo en TWIN.')}</p></div>
<div><h2>SOUND LAB / SYSEX</h2><p class="small">{T('Reads the four documented blocks of the <b>active</b> patch (patch, distortion, EFX 1, EFX 2), saves them as a .json backup and restores them. FX type and parameter writes ask before they go out.', 'Lee los cuatro bloques documentados del patch <b>activo</b> (patch, distorsión, EFX 1, EFX 2), los guarda como respaldo .json y los restaura. Las escrituras de tipo y parámetro de FX preguntan antes de salir.')}</p></div></div>
{warn(T('Restore always asks first, then reads the current sound and downloads it as a safety backup, and only then writes, and only to the active (edit) patch, never to a saved user slot. Only the official TB-3 MIDI implementation is used. No TB-3? Tick DEMO to try everything with a simulated one.', 'Restaurar siempre pregunta primero, luego lee el sonido actual y lo descarga como respaldo de seguridad, y solo entonces escribe, y solo en el patch activo (de edición), nunca en un slot de usuario guardado. Solo se usa la implementación MIDI oficial de la TB-3. ¿Sin TB-3? Marca DEMO para probar todo con una simulada.'))}''')

    # ------------------------------------------------------------ troubleshooting
    page(T('Troubleshooting', 'Solución de problemas'), f'''{head('13', T('TROUBLESHOOTING', 'PROBLEMAS'), T('If something <em>goes quiet.</em>', 'Si algo <em>se calla.</em>'))}
{table([T('Problem', 'Problema'), T('Try this', 'Prueba esto')], [
 [T('No sound', 'No suena'), T('press PLAY once more (browsers need a click), raise the system volume, check that TWIN LINK is not on HARDWARE and MUTE is off, check VOLUME.', 'presiona PLAY otra vez (los navegadores necesitan un clic), sube el volumen del sistema, revisa que TWIN LINK no esté en HARDWARE y que MUTE esté apagado, revisa VOLUMEN.')],
 [T('Clicks or crackles', 'Clics o chasquidos'), T('QUALITY → ECO 1×, close other tabs, plug the laptop in. On phones ECO is recommended.', 'CALIDAD → ECO 1×, cierra otras pestañas, conecta la laptop a la corriente. En teléfonos se recomienda ECO.')],
 [T('Silent on iPhone', 'Sin sonido en iPhone'), T('turn off the silent switch; Safari plays web audio through the ringer channel.', 'apaga el interruptor de silencio; Safari reproduce el audio web por el canal del timbre.')],
 [T('CONNECT USB MIDI does nothing', 'CONECTAR USB MIDI no hace nada'), T('use Chrome or Edge on https or localhost, allow MIDI in the address bar, quit other apps using the TB-3 port, try another cable (data, not charge-only).', 'usa Chrome o Edge en https o localhost, permite MIDI en la barra de dirección, cierra otras apps que usen el puerto de la TB-3, prueba otro cable (de datos, no solo de carga).')],
 [T('TB-3 plays double notes', 'La TB-3 toca notas dobles'), T('only one sender at a time: the browser OR the DAW. Check the MIDI channel.', 'una sola fuente a la vez: el navegador O el DAW. Revisa el canal MIDI.')],
 [T('Read patch gives nothing', 'Leer patch no da nada'), T('SysEx must be allowed: reload, press CONNECT again and accept SysEx.', 'SysEx debe estar permitido: recarga, presiona CONECTAR otra vez y acepta SysEx.')],
 [T('Shadow Capture is empty', 'Shadow Capture sale vacío'), T('the TB-3 must transmit MIDI notes on the channel you chose; check its MIDI settings.', 'la TB-3 debe transmitir notas MIDI en el canal que elegiste; revisa su configuración MIDI.')],
 [T('DRAG MIDI does nothing', 'ARRASTRA MIDI no hace nada'), T('dragging files out works in Chrome / Edge. Elsewhere press ↓ MIDI and drag the downloaded file.', 'arrastrar archivos funciona en Chrome / Edge. En otros navegadores presiona ↓ MIDI y arrastra el archivo descargado.')],
 [T('My saved patterns are gone', 'Mis patrones guardados desaparecieron'), T('browser storage is per browser and is erased with site data. Keep a ↓ PROJECT .JSON of anything important.', 'el almacenamiento es por navegador y se borra con los datos del sitio. Guarda un ↓ PROYECTO .JSON de todo lo importante.')],
 [T('The page looks old after an update', 'La página se ve vieja tras una actualización'), T('reload with Ctrl/⌘ + Shift + R.', 'recarga con Ctrl/⌘ + Shift + R.')]], 'tight')}
<p class="small">{T(f'Still stuck? Open an issue at <a href="{REPO}/issues">github.com/drophlex/DFD-ACID-ENGINE/issues</a> with the line from the LINK MONITOR.', f'¿Sigues atorado? Abre un issue en <a href="{REPO}/issues">github.com/drophlex/DFD-ACID-ENGINE/issues</a> con la línea del MONITOR DEL LINK.')}</p>''')

    # ------------------------------------------------------------ recipes
    rec = [(T('CLASSIC 303-STYLE HOUSE LINE', 'LÍNEA HOUSE CLÁSICA'), [T('STYLE ACID HOUSE, KEY A1, MINOR, 124 BPM', 'ESTILO ACID HOUSE, TONO A1, MENOR, 124 BPM'), T('RESONANCE 80 %, ENV MOD 60 %, DECAY 400 ms', 'RESONANCIA 80 %, ENV MOD 60 %, CAÍDA 400 ms'), T('DELAY on, time 3/16, mix 20 %', 'DELAY encendido, tiempo 3/16, mezcla 20 %'), T('Sweep CUTOFF slowly over 16 bars', 'Barre el CORTE despacio durante 16 compases')]),
           (T('PEAK-TIME TECHNO', 'TECHNO DE HORA PICO'), [T('STYLE ACID TECHNO, 138 BPM, PHRYGIAN', 'ESTILO ACID TECHNO, 138 BPM, FRIGIO'), T('DRIVE 60 %, OVERDRIVE 30 %, COMPRESSOR ratio 6', 'DRIVE 60 %, OVERDRIVE 30 %, COMPRESOR ratio 6'), T('Copy A → B, mutate B 40 %', 'Copia A → B, muta B 40 %'), T('Ride the XY from A to B; SQUELCH before the drop', 'Recorre el XY de A a B; SQUELCH antes del drop')]),
           (T('FILTER HOUSE PUMP', 'FILTER HOUSE QUE BOMBEA'), [T('STYLE FILTER HOUSE, 122 BPM, DORIAN, SWING 20 %', 'ESTILO FILTER HOUSE, 122 BPM, DÓRICO, SWING 20 %'), T('SAW ⇄ SQUARE halfway, SUB 40 %', 'ONDA a la mitad, SUB 40 %'), T('DYN FILTER on, cycle 16 steps, depth 60 %', 'FILTRO DIN encendido, ciclo 16 pasos, profundidad 60 %'), T('Lock RHYTHM, generate new notes per section', 'Bloquea RITMO, genera notas nuevas por sección')]),
           (T('BRAINDANCE SCRIBBLE', 'GARABATO BRAINDANCE'), [T('STYLE BRAINDANCE, 168 BPM, CHROMATIC, CHAOS 70 %', 'ESTILO BRAINDANCE, 168 BPM, CROMÁTICA, CAOS 70 %'), T('EXPERT: PROBABILITY 40 %, CONDITIONS 40 %', 'EXPERTO: PROBABILIDAD 40 %, CONDICIONES 40 %'), T('CRUSH on, bits 6; RING MOD mix 20 %', 'CRUSH encendido, bits 6; RING MOD mezcla 20 %'), T('Hold STUTTER on the last beat of each 4 bars', 'Mantén STUTTER en el último tiempo de cada 4 compases')])]
    page(T('Recipes', 'Recetas'), f'''{head('14', T('RECIPES', 'RECETAS'), T('Four starting <em>points.</em>', 'Cuatro puntos <em>de partida.</em>'))}
<div class="grid2">{''.join(f'<div class="card recipe {c}"><h3>{n}</h3><ol>{"".join(f"<li>{x}</li>" for x in st)}</ol></div>' for (n, st), c in zip(rec, ['', 'v', 'o', 'b']))}</div>
{tip(T('Every recipe is a start, not a rule: generate a few seeds in each style and lock the genes you like.', 'Cada receta es un comienzo, no una regla: genera varias semillas en cada estilo y bloquea los genes que te gusten.'))}
{img('top', T('<b>fig. 12</b> · DFD / NIGHT SIGNAL, the first factory pattern, running.', '<b>fig. 12</b> · DFD / NIGHT SIGNAL, el primer patrón de fábrica, sonando.'), '88%')}''')

    # ------------------------------------------------------------ credits / back
    page(T('Credits and support', 'Créditos y apoyo'), f'''{head('15', T('CREDITS', 'CRÉDITOS'), T('Free, open, <em>made in Mérida.</em>', 'Libre, abierto, <em>hecho en Mérida.</em>'))}
<p class="lead">{T('ACID ENGINE is free software under the GNU GPL-3.0: use it, study it, change it and share it. The engine, the generator, the factory patterns and this manual are original work.', 'ACID ENGINE es software libre bajo la GNU GPL-3.0: úsalo, estúdialo, cámbialo y compártelo. El motor, el generador, los patrones de fábrica y este manual son obra original.')}</p>
{table([T('Part', 'Parte'), T('Detail', 'Detalle')], [
 [T('Engine', 'Motor'), T('one C++17 file (engine/dfd_acid.h) compiled to WebAssembly, running in an AudioWorklet', 'un archivo C++17 (engine/dfd_acid.h) compilado a WebAssembly, corriendo en un AudioWorklet')],
 [T('Source code', 'Código fuente'), f'<a href="{REPO}">github.com/drophlex/DFD-ACID-ENGINE</a>'],
 [T('Website', 'Sitio'), f'<a href="{WEB}">drophlex.github.io/DFD-ACID-ENGINE</a>'],
 [T('Coming next', 'Lo que sigue'), T('VST3 / AU plugin for Mac and Windows, and an M-VAVE FM-1 firmware, each with its own manual', 'plugin VST3 / AU para Mac y Windows, y un firmware para el M-VAVE FM-1, cada uno con su manual')]], 'tight')}
<div class="back"><div class="cmark small"><span>D</span><span>F</span><span>D</span></div>
<div class="btitle">ACID <em>ENGINE</em></div>
<p>{CREDIT}<br><a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a></p>
<a class="paypal" href="{DON}">♥ {T('Keep it free · Donate with PayPal', 'Mantenlo gratis · Dona con PayPal')}</a>
<p class="small">{T('Independent software, not affiliated with or endorsed by Roland Corporation. Roland and TB-3 are trademarks of their owner, used only to describe compatibility. M-VAVE and FM-1 are trademarks of their owner. No warranty.', 'Software independiente, sin afiliación ni respaldo de Roland Corporation. Roland y TB-3 son marcas de su dueño, usadas solo para describir compatibilidad. M-VAVE y FM-1 son marcas de su dueño. Sin garantía.')}</p></div>''')

CSS = open(os.path.join(HERE, 'manual.css')).read()
BOTH = False
OFF = 0
EDITION_NAME, EDITION_VER = 'WEB', '1.1'
def render(langs, out, builder=None, label='WEB MANUAL', label_es='MANUAL WEB'):
    global L, BOTH, OFF
    BOTH = len(langs) > 1; allp = []; npl = 0
    for lang in langs:
        L = lang; OFF = npl; (builder or build_web)(); off = npl
        for i, (cls, body, title) in enumerate(PAGES):
            n = i + 1 + off
            foot = '' if cls == 'coverp' else f'<footer><span>DFD / ACID ENGINE · {T(label, label_es)} · {lang.upper()}</span><span>{title}</span><span class="pn">{n:02d}</span></footer>'
            allp.append(f'<section class="page {cls}"><div class="rail"></div>{body}{foot}</section>')
        npl += len(PAGES)
    open(out, 'w').write(f'<!DOCTYPE html><html><head><meta charset="utf-8"><title>DFD ACID ENGINE · Manual</title><style>{CSS}</style></head><body>{"".join(allp)}</body></html>')

async def pdf(jobs):
    from playwright.async_api import async_playwright
    async with async_playwright() as p:
        b = await p.chromium.launch(); pg = await b.new_page()
        for src, name in jobs:
            await pg.goto('file://' + src); await pg.wait_for_timeout(700)
            over = await pg.evaluate('''() => [...document.querySelectorAll('.page')].map((pg, i) => { const r = pg.getBoundingClientRect(); let m = 0; pg.querySelectorAll('*').forEach(e => { if (e.closest('footer') || e.closest('.cover') || e.classList.contains('rail')) return; const q = e.getBoundingClientRect(); if (q.height) m = Math.max(m, q.bottom - r.top); }); return [i + 1, Math.round(m)]; }).filter(x => x[1] > 985)''')
            await pg.pdf(path=os.path.join(OUT, name), width='8.5in', height='11in', print_background=True)
            print(name, 'overflow:', over)
        await b.close()

from manual_more import build_plugin, build_fm1, setup
setup(globals())
EDITIONS = [('Web', build_web, 'WEB', '1.1', 'WEB MANUAL', 'MANUAL WEB'), ('Plugin', build_plugin, 'VST3 · AU · APP', '1.1 BETA', 'PLUG-IN MANUAL', 'MANUAL DEL PLUG-IN'),
            ('FM1', build_fm1, 'M-VAVE FM-1', '1.8.2-da1 BETA', 'FM-1 MANUAL', 'MANUAL FM-1')]
jobs = []
only = sys.argv[1:] or [e[0] for e in EDITIONS]
for ed, fn, ename, ever, lab, lab_es in EDITIONS:
    if ed not in only: continue
    EDITION_NAME, EDITION_VER = ename, ever
    for langs, suf in ((['en'], 'EN'), (['es'], 'ES'), (['en', 'es'], 'EN-ES')):
        name = f'DFD-ACID-ENGINE-{ed}-Manual-{suf}'
        src = os.path.join(BUILD, name + '.html'); render(langs, src, fn, lab, lab_es); jobs.append((src, name + '.pdf'))
asyncio.run(pdf(jobs))
