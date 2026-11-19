# The plug-in's page with a mocked JUCE backend: checks the native-engine path (messages, project restore, save, drag MIDI).
import asyncio, json, sys
from playwright.async_api import async_playwright
URL = sys.argv[1] if len(sys.argv) > 1 else 'http://localhost:8811/plugin/ui/index.html'
MOCK = r'''
window.__mock = {sent: [], calls: [], project: null};
const L = {};
window.__JUCE__ = {backend: {
  addEventListener(n, f) { (L[n] = L[n] || []).push(f); return [n, f]; },
  removeEventListener(t) { if (t) L[t[0]] = (L[t[0]] || []).filter(x => x !== t[1]); },
  emitEvent(n, d) {
    if (n !== '__juce__invoke') return;
    window.__mock.calls.push(d.name);
    let r = null;
    if (d.name === 'msg') window.__mock.sent.push(JSON.parse(d.params[0]));
    if (d.name === 'getProject') r = window.__mock.project || {params: [], A: '', B: '', ui: ''};
    if (d.name === 'saveFile' || d.name === 'dragMidi') r = true;
    setTimeout(() => (L['__juce__complete'] || []).forEach(f => f({promiseId: d.resultId, result: r})), 5);
  }}};
window.__fire = (n, e) => (L[n] || []).forEach(f => f(e));
'''
async def main():
    async with async_playwright() as p:
        b = await p.chromium.launch(); pg = await b.new_page(viewport={'width': 1440, 'height': 880}); errs = []
        pg.on('pageerror', lambda e: errs.append(str(e)))
        await pg.add_init_script(MOCK)
        await pg.goto(URL); await pg.wait_for_timeout(1200)
        st = await pg.evaluate("({plugin: document.body.classList.contains('plugin'), sent: __mock.sent.length, kinds: [...new Set(__mock.sent.map(m => m.t))], calls: [...new Set(__mock.calls)]})")
        print('fresh instance:', st); assert st['plugin'] and 'pat' in st['kinds'] and 'p' in st['kinds']
        await pg.click('#play'); await pg.wait_for_timeout(200)
        await pg.evaluate("__fire('tele', {tele: [1, 3, 0, .3, 900, 0, 36, 3], scope: new Array(256).fill(0), w: 0, params: DFD.params.map((v, i) => i === DFD.ID.cutoff ? 2000 : v)})")
        await pg.wait_for_timeout(100)
        cut = await pg.evaluate("DFD.params[DFD.ID.cutoff]"); print('host automation reaches the page: cutoff', cut); assert abs(cut - 2000) < 1
        ph = await pg.evaluate("document.querySelectorAll('.step.playhead').length"); assert ph == 1, 'playhead'
        await pg.click('#exportMidi'); await pg.wait_for_timeout(800)
        await pg.dispatch_event('#dragMidi', 'pointerdown'); await pg.wait_for_timeout(800)
        calls = await pg.evaluate("__mock.calls"); print('calls', sorted(set(calls))); assert 'saveFile' in calls and 'dragMidi' in calls and 'setUi' in calls
        # restore: a saved project comes back exactly
        proj = await pg.evaluate("""(() => { const toB64 = b => btoa(String.fromCharCode(...b)); const mk = s => { const b = new Uint8Array(4 + s.length * 8); new DataView(b.buffer).setInt32(0, s.length, true); s.forEach((x, k) => { b.set([x, 1, 60, 100, 1, 100, 0, 0], 4 + k * 8); }); return toB64(b); };
          return {params: DFD.params.map((v, i) => i === DFD.ID.res ? .91 : v), A: mk([40, 41, 42, 43, 44, 45, 46, 47]), B: mk([50, 51, 52, 53, 54, 55, 56, 57]), ui: JSON.stringify({v: 1, name: 'RESTORED', edit: 'B', gen: {mode: 'dirty', root: 33, scale: 'minor', seed: 42}})}; })()""")
        p2 = await b.new_page(); await p2.add_init_script(MOCK + 'window.__mock.project = ' + json.dumps(proj) + ';')
        p2.on('pageerror', lambda e: errs.append(str(e)))
        await p2.goto(URL); await p2.wait_for_timeout(1200)
        r = await p2.evaluate("({len: DFD.pats.A.length, a0: DFD.pats.A[0].note, b7: DFD.pats.B[7].note, res: DFD.params[DFD.ID.res], name: document.getElementById('customName').value, mode: document.getElementById('mode').value, tag: document.getElementById('abTag').textContent})")
        print('restored:', r); assert r['len'] == 8 and r['a0'] == 40 and r['b7'] == 57 and abs(r['res'] - .91) < 1e-4 and r['name'] == 'RESTORED' and r['mode'] == 'dirty' and r['tag'] == 'B'
        await pg.screenshot(path='/tmp/shots/plugin_page.png')
        print('errors', errs); assert not errs
        await b.close(); print('PLUGIN PAGE OK')
asyncio.run(main())
