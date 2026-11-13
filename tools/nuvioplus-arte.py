#!/usr/bin/env python3
"""Nuvio+ brand art: the logo with "NUVIO+" written underneath.

Builds every package icon, the Android TV banner, the gallery "Original" icon
and the in-app opening mark from two sources that ship in the repo:
  deploy/app/art/marcas/logo-novo-marca.png  (the logo, cropped from the top)
  deploy/app/fonts/Montserrat-Bold.ttf       (the NUVIO+ lettering)

Outputs (idempotent; run from anywhere):
  design/marca/nuvio-plus-icone.png                 1024 master, dark tile
  deploy/app/icon.png, icon-20.png                  80x80   LG webOS
  deploy/app/icon-large.png, icon-large-20.png      130x130 LG webOS
  deploy/app/tizen/icon.png                         512x512 Samsung
  android/.../mipmap-nodpi/ic_launcher.png          192x192 Android TV launcher
  android/.../drawable-nodpi/banner.png             320x180 Android TV banner
  deploy/app/art/icones-app/original.png            256x256 gallery, transparent
  deploy/app/art/marcas/logo-novo-marca.png         1000x746 opening mark, transparent
  deploy/app/art/marcas/logo-novo-horizontal.png    x201 About lockup, transparent
  src/iconeapp_arte.h  (the Original hash, same recipe as tools/icones-app.sh)

The logo source is read BEFORE logo-novo-marca.png is rewritten, from a pristine
copy kept at design/marca/nuvio-logo-simbolo.png (created on the first run).

Requires Pillow.
"""
import hashlib
import os
import sys

from PIL import Image, ImageDraw, ImageFont

RAIZ = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
os.chdir(RAIZ)

TILE = (11, 15, 26, 255)          # the launcher tile of the current icons (#0B0F1A)
GRAD = [(0.0, (28, 230, 247)), (0.5, (46, 134, 248)), (1.0, (198, 83, 248))]
FONTE = "deploy/app/fonts/Montserrat-Bold.ttf"
LOGO_FONTE = "design/marca/nuvio-logo-simbolo.png"
RES = "android/app/src/main/res"


def logo_fonte():
    """The logo alone (no lettering), cut from the old opening mark once."""
    if not os.path.exists(LOGO_FONTE):
        m = Image.open("deploy/app/art/marcas/logo-novo-marca.png").convert("RGBA")
        topo = m.crop((0, 0, m.width, 610))           # the lettering starts below ~620
        topo = topo.crop(topo.getbbox())
        os.makedirs(os.path.dirname(LOGO_FONTE), exist_ok=True)
        topo.save(LOGO_FONTE, optimize=True)
    return Image.open(LOGO_FONTE).convert("RGBA")


def gradiente(w, h):
    g = Image.new("RGBA", (w, h))
    px = g.load()
    for y in range(h):
        for x in range(w):
            t = (x + y) / max(1, (w + h - 2))
            for i in range(len(GRAD) - 1):
                t0, c0 = GRAD[i]
                t1, c1 = GRAD[i + 1]
                if t <= t1:
                    u = (t - t0) / (t1 - t0)
                    px[x, y] = tuple(int(c0[k] + (c1[k] - c0[k]) * u) for k in range(3)) + (255,)
                    break
    return g


def letreiro(px_fonte, track=0.12):
    """NUVIO in white + a gradient "+", tracked, trimmed to its ink."""
    f = ImageFont.truetype(FONTE, px_fonte)
    pad = px_fonte
    larg = int(px_fonte * 8)
    alt = int(px_fonte * 2)
    branco = Image.new("L", (larg, alt), 0)
    mais = Image.new("L", (larg, alt), 0)
    db, dm = ImageDraw.Draw(branco), ImageDraw.Draw(mais)
    x = pad
    for ch in "NUVIO+":
        (dm if ch == "+" else db).text((x, pad // 2), ch, font=f, fill=255)
        x += f.getlength(ch) + (track * px_fonte if ch != "+" else 0)
    out = Image.new("RGBA", (larg, alt), (0, 0, 0, 0))
    out.paste((255, 255, 255, 255), (0, 0), branco)
    caixa = mais.getbbox()
    if caixa:
        g = gradiente(caixa[2] - caixa[0], caixa[3] - caixa[1])
        camada = Image.new("RGBA", (larg, alt), (0, 0, 0, 0))
        camada.paste(g, caixa[:2])
        out.paste(camada, (0, 0), mais)
    return out.crop(out.getbbox())


def pilha(largura_logo, escala_texto=0.2237, vao=0.1447, desloc=0.066):
    """Logo over NUVIO+ at a given logo width (proportions of the approved mockup:
    logo 152 px, letters 34 px, gap 22 px, logo nudged 10 px right)."""
    lg = logo_fonte()
    alt = round(largura_logo * lg.height / lg.width)
    lg = lg.resize((largura_logo, alt), Image.LANCZOS)
    tx = letreiro(round(largura_logo * escala_texto))
    gap = round(largura_logo * vao)
    nudge = round(largura_logo * desloc)
    w = max(lg.width + nudge * 2, tx.width)
    h = lg.height + gap + tx.height
    c = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    c.alpha_composite(lg, ((w - lg.width) // 2 + nudge, 0))
    c.alpha_composite(tx, ((w - tx.width) // 2, lg.height + gap))
    return c


def quadro(lado, fundo, frac_alt=0.70, so_logo=False):
    """Square art: the stack (or the logo alone) centred, `frac_alt` of the side tall."""
    S = 4                                   # draw big, shrink once
    L = lado * S
    tela = Image.new("RGBA", (L, L), fundo if fundo else (0, 0, 0, 0))
    if so_logo:
        lg = logo_fonte()
        h = round(L * frac_alt)
        w = round(h * lg.width / lg.height)
        lg = lg.resize((w, h), Image.LANCZOS)
        tela.alpha_composite(lg, ((L - w) // 2 + round(w * 0.05), (L - h) // 2))
    else:
        p = pilha(round(L * 0.475))
        k = (L * frac_alt) / p.height
        if p.width * k > L * 0.86:
            k = (L * 0.86) / p.width
        p = p.resize((round(p.width * k), round(p.height * k)), Image.LANCZOS)
        tela.alpha_composite(p, ((L - p.width) // 2, (L - p.height) // 2))
    return tela.resize((lado, lado), Image.LANCZOS)


def salvar(img, caminho, fundo_solido=False):
    os.makedirs(os.path.dirname(caminho), exist_ok=True)
    if fundo_solido:
        img = img.convert("RGB")
    img.save(caminho, optimize=True)
    print("  ", caminho, img.size)


def main():
    logo_fonte()                            # keep the pristine logo before rewriting the mark
    salvar(quadro(1024, TILE), "design/marca/nuvio-plus-icone.png", True)
    for lado, nomes in ((80, ["deploy/app/icon.png", "deploy/app/icon-20.png"]),
                        (130, ["deploy/app/icon-large.png", "deploy/app/icon-large-20.png"]),
                        (512, ["deploy/app/tizen/icon.png"]),
                        (192, [RES + "/mipmap-nodpi/ic_launcher.png"])):
        img = quadro(lado, TILE)
        for n in nomes:
            salvar(img, n, True)
    salvar(quadro(256, None, 0.78), "deploy/app/art/icones-app/original.png")

    # Android TV banner 320x180: the same stack, 75% of the height.
    S = 4
    b = Image.new("RGBA", (320 * S, 180 * S), TILE)
    p = pilha(84 * S)
    k = (180 * S * 0.75) / p.height
    p = p.resize((round(p.width * k), round(p.height * k)), Image.LANCZOS)
    b.alpha_composite(p, ((b.width - p.width) // 2, (b.height - p.height) // 2))
    salvar(b.resize((320, 180), Image.LANCZOS), RES + "/drawable-nodpi/banner.png", True)

    # Opening mark (src/abertura.c, logo Novo): 1000x746, transparent, like before.
    m = Image.new("RGBA", (1000, 746), (0, 0, 0, 0))
    p = pilha(500)
    k = min(730 / p.height, 980 / p.width)
    p = p.resize((round(p.width * k), round(p.height * k)), Image.LANCZOS)
    m.alpha_composite(p, ((1000 - p.width) // 2, (746 - p.height) // 2))
    salvar(m, "deploy/app/art/marcas/logo-novo-marca.png")

    # About / What's new lockup (LOGO_F_HORIZONTAL): logo and NUVIO+ side by side,
    # 201 px tall like before, trimmed to the art (drawn aspect-fit by the app).
    S = 4
    H = 201 * S
    lg = logo_fonte()
    lw = round(H * lg.width / lg.height)
    lg = lg.resize((lw, H), Image.LANCZOS)
    tx = letreiro(round(H * 0.50), track=0.10)
    k = (H * 0.36) / tx.height                 # cap height ~36% of the logo, as before
    tx = tx.resize((round(tx.width * k), round(tx.height * k)), Image.LANCZOS)
    gap = round(H * 0.24)
    hz = Image.new("RGBA", (lw + gap + tx.width + 4 * S, H), (0, 0, 0, 0))
    hz.alpha_composite(lg, (0, 0))
    hz.alpha_composite(tx, (lw + gap, (H - tx.height) // 2))
    hz = hz.resize((round(hz.width / S), 201), Image.LANCZOS)
    salvar(hz, "deploy/app/art/marcas/logo-novo-horizontal.png")

    # Original's fingerprint, the recipe of tools/icones-app.sh.
    h = hashlib.sha256()
    for n in ("deploy/app/art/icones-app/original.png", "deploy/app/icon.png",
              "deploy/app/icon-large.png", "deploy/app/tizen/icon.png"):
        h.update(open(n, "rb").read())
    novo = h.hexdigest()[:12]
    cab = "src/iconeapp_arte.h"
    linhas = open(cab).read().split("\n")
    for i, l in enumerate(linhas):
        if l.startswith("static const char *const ARTE_HASH"):
            linhas[i + 1] = '  "%s",' % novo
            break
    open(cab, "w").write("\n".join(linhas))
    print("   original hash", novo)


if __name__ == "__main__":
    sys.exit(main())
