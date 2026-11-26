# Host preview of font.c layout logic (same rules) -> PNG for visual check.
import sys, re
from PIL import Image
src = open('src/font_data.c', encoding='utf-8').read()
rows = re.findall(r'\{0x([0-9A-F]{4}), (\d+), (-?\d+), (-?\d+), (\d+), (\d+), (\d), (\d+)\}', src)
bmp = bytes(int(x) for x in re.findall(r'\d+', src.split('font_bitmap[')[1].split('};')[0].split('{',1)[1]))
G = {int(r[0],16): dict(adv=int(r[1]),x0=int(r[2]),y0=int(r[3]),w=int(r[4]),h=int(r[5]),mark=r[6]=='1',off=int(r[7])) for r in rows}
text = sys.argv[1]; out = sys.argv[2]
W, H = 300, 40
img = Image.new('RGB', (W, H), (255,255,255)); px = img.load()
def draw(g, ox, by):
    for j in range(g['h']):
        for i in range(g['w']):
            a = bmp[g['off'] + j*g['w'] + i]
            x, y = ox + g['x0'] + i, by + g['y0'] + j
            if a and 0 <= x < W and 0 <= y < H:
                px[x, y] = (255-a, 255-a, 255-a)
pen = base_x = 10; base_adv = 0; by = 28
for ch in text:
    g = G.get(ord(ch))
    if not g: continue
    if g['mark']:
        draw(g, base_x + base_adv//2 - (g['x0'] + g['w']//2), by)
    else:
        draw(g, pen, by); base_x = pen; base_adv = g['adv']; pen += g['adv']
img.resize((W*3, H*3), Image.NEAREST).save(out)
print('saved', out)
