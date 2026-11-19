# Browser end-to-end smoke test: python3 tests/e2e/smoke.py http://localhost:8811/web/  (needs: pip install playwright; playwright install chromium)
import asyncio, json, sys, os
from playwright.async_api import async_playwright
URL=sys.argv[1] if len(sys.argv)>1 else 'http://localhost:8811/'
SHOTS=os.environ.get('SHOTS','/tmp/shots'); os.makedirs(SHOTS,exist_ok=True)
async def main():
  async with async_playwright() as p:
    b=await p.chromium.launch(args=['--autoplay-policy=no-user-gesture-required'])
    pg=await b.new_page(viewport={'width':1440,'height':900})
    errs=[]; pg.on('console', lambda m: errs.append(m.type+': '+m.text) if m.type in ('error','warning') else None); pg.on('pageerror', lambda e: errs.append('PAGEERROR '+str(e)))
    await pg.goto(URL); await pg.wait_for_timeout(800)
    print('monitor:', await pg.text_content('#monitor'))
    await pg.click('#play'); await pg.wait_for_timeout(2500)
    t=await pg.evaluate('({tele:DFD.eng.tele, state:DFD.eng.ctx.state, sr:DFD.eng.ctx.sampleRate})'); print('tele',json.dumps(t))
    ph=await pg.evaluate('document.querySelectorAll(".step.playhead").length'); print('playhead cells', ph)
    await pg.screenshot(path=SHOTS+'/desk_easy.png', full_page=True)
    await pg.click('#layerExpert'); await pg.click('#langES'); await pg.wait_for_timeout(300)
    await pg.screenshot(path=SHOTS+'/desk_expert_es.png', full_page=True)
    # generate + exports
    await pg.click('#generate'); await pg.wait_for_timeout(300)
    async with pg.expect_download() as d: await pg.click('#exportMidi')
    dl=await d.value; path=await dl.path(); data=open(path,'rb').read(); print('midi', dl.suggested_filename, len(data), data[:4]); assert data[:4]==b'MThd' and len(data)>200
    async with pg.expect_download() as d: await pg.click('#exportWav')
    dl=await d.value; path=await dl.path(); data=open(path,'rb').read(); print('wav', dl.suggested_filename, len(data), data[:4], data[8:12]); assert data[:4]==b'RIFF' and len(data)>1000000
    await pg.click('#stop'); await pg.wait_for_timeout(200)
    print('errors:', errs)
    assert not errs, errs
    assert t['state']=='running' and t['tele'][0]==1 and t['tele'][3]>0.01, 'no sound'
    assert ph==1, 'no playhead'
    m=await b.new_page(viewport={'width':390,'height':844}); await m.goto(URL); await m.wait_for_timeout(800); await m.screenshot(path=SHOTS+'/mobile.png', full_page=False)
    await b.close()
    print('E2E OK')
asyncio.run(main())
