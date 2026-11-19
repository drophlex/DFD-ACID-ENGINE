#!/usr/bin/env python3
"""One standalone HTML file (works from disk, no server): every module goes in an import map as a data: URL."""
from pathlib import Path
import base64, json, re
root = Path(__file__).resolve().parent
web = root / 'web'
html = (web / 'index.html').read_text(encoding='utf-8')
css = (web / 'styles.css').read_text(encoding='utf-8')
bare = lambda src: re.sub(r"from '\./([\w-]+)\.js'", r"from 'dfd-\1'", src)
mods = {}
for name in ('dfd-wasm.js', 'i18n.js', 'pattern.js', 'engine.js', 'tb3-midi.js'):
    mods['dfd-' + name[:-3]] = 'data:text/javascript;base64,' + base64.b64encode(bare((web / name).read_text(encoding='utf-8')).encode()).decode()
imap = '<script type="importmap">' + json.dumps({'imports': mods}) + '</script>'
html = html.replace('<link rel="stylesheet" href="styles.css">', '<style>' + css + '</style>' + imap)
html = html.replace('<script type="module" src="main.js"></script>', '<script type="module">\n' + bare((web / 'main.js').read_text(encoding='utf-8')) + '\n</script>')
out = root / 'DFD_ACID_ENGINE_OFFLINE.html'
out.write_text(html, encoding='utf-8')
print(f'{out.name}: {out.stat().st_size:,} bytes')
