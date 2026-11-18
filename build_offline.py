#!/usr/bin/env python3
"""Produce one standalone browser document from original local web sources (no bundler dependency)."""
from pathlib import Path
import re
root=Path(__file__).resolve().parent
base=(root/'web'/'index.html').read_text()
styles=(root/'web'/'styles.css').read_text()
sources=[]
for name in ('pattern.js','audio.js','tb3-midi.js','main.js'):
    body=(root/'web'/name).read_text()
    body=re.sub(r'(?m)^import .*? from [\'"].*?[\'"];\s*$', '', body)
    body=re.sub(r'(?m)^export ', '', body)
    sources.append('// '+name+'\n'+body)
base=base.replace('<link rel="stylesheet" href="styles.css">','<style>'+styles+'</style>')
base=base.replace('<script type="module" src="main.js"></script>','<script type="module">\n'+'\n'.join(sources)+'\n</script>')
p=root/'DFD_ACID_ENGINE_OFFLINE.html'
p.write_text(base)
print(f'{p}: {p.stat().st_size:,} bytes')
