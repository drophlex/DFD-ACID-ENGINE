# Screenshots for the manual, in English and Spanish:  python3 tools/manual/shots.py  (page served at localhost:8811)
import asyncio, os
from playwright.async_api import async_playwright
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'build', 'manual', 'shots')
os.makedirs(OUT, exist_ok=True)
MOCK = r'''
const L = {};
window.__JUCE__ = {backend: {addEventListener(n, f) { (L[n] = L[n] || []).push(f); return [n, f]; }, removeEventListener() {},
  emitEvent(n, d) { if (n !== '__juce__invoke') return; let r = null; if (d.name === 'getProject') r = {params: [], A: '', B: '', ui: ''};
    setTimeout(() => (L['__juce__complete'] || []).forEach(f => f({promiseId: d.resultId, result: r})), 5); }}};
window.__fire = (n, e) => (L[n] || []).forEach(f => f(e));
'''
async def main():
    async with async_playwright() as p:
        b = await p.chromium.launch(args=['--autoplay-policy=no-user-gesture-required'])
        for lang in ('en', 'es'):
            ctx = await b.new_context(viewport={'width': 1440, 'height': 900}, device_scale_factor=2)
            pg = await ctx.new_page(); errs = []; pg.on('pageerror', lambda e: errs.append(str(e)))
            await ctx.add_init_script(f"try{{localStorage.clear();localStorage.setItem('dfd-acid-lang','{lang}');localStorage.setItem('dfd-acid-layer','easy')}}catch(e){{}}")
            await pg.goto('http://localhost:8811/web/'); await pg.wait_for_timeout(900)
            D = os.path.join(OUT, lang + '_')
            await pg.click('#play'); await pg.wait_for_timeout(1600)
            await pg.screenshot(path=D + 'top.png')
            el = lambda s: pg.query_selector(s)
            await (await el('.toolbar')).screenshot(path=D + 'toolbar.png')
            await (await el('.workbench')).screenshot(path=D + 'easy.png')
            await (await el('.generator')).screenshot(path=D + 'gen_easy.png')
            await pg.click('#layerExpert'); await pg.wait_for_timeout(250)
            await pg.evaluate("document.querySelectorAll('.step-index')[4].click()"); await pg.wait_for_timeout(200)
            await (await el('.generator')).screenshot(path=D + 'gen_expert.png')
            await (await el('#grid')).screenshot(path=D + 'grid.png')
            await (await el('#inspector')).screenshot(path=D + 'inspector.png')
            await (await el('.sub-panel')).screenshot(path=D + 'sound.png')
            await (await el('.utilities')).screenshot(path=D + 'archive.png')
            await pg.evaluate("DFD.setP('dly_on',1);DFD.setP('rev_on',1);DFD.setP('crush_on',1)")
            await (await el('.fxrack')).screenshot(path=D + 'fx.png')
            await pg.click('#dirtyGo'); await pg.wait_for_timeout(600)
            await (await el('#dirtyLab')).screenshot(path=D + 'lab.png')
            await (await el('.perform')).screenshot(path=D + 'perform.png')
            await pg.check('#demo'); await pg.select_option('#hardwareMode', 'twin'); await pg.wait_for_timeout(300)
            await (await el('.hardware')).screenshot(path=D + 'tb3.png')
            await pg.click('#stop')
            print(lang, errs); await ctx.close()
            # the plug-in's window (the same page in the DAW's web view, with a stand-in for JUCE's backend)
            ctx = await b.new_context(viewport={'width': 1440, 'height': 880}, device_scale_factor=2)
            await ctx.add_init_script(f"try{{localStorage.clear();localStorage.setItem('dfd-acid-lang','{lang}');localStorage.setItem('dfd-acid-layer','easy')}}catch(e){{}}" + MOCK)
            pg = await ctx.new_page(); await pg.goto('http://localhost:8811/DFD_ACID_ENGINE_OFFLINE.html'); await pg.wait_for_timeout(1200)
            await pg.click('#play'); await pg.wait_for_timeout(300)
            await pg.evaluate("__fire('tele', {tele: [1, 5, 0, .3, 900, 0, 36, 5], scope: Array.from({length: 256}, (_, i) => Math.sin(i / 9) * .5), w: 0})")
            await pg.wait_for_timeout(200); await pg.screenshot(path=D + 'plugin.png')
            await ctx.close()
        await b.close()
asyncio.run(main())
