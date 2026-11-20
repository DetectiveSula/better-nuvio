// Nuvio+ 1.0.12 (19 Nov 2026): the Apple TV-style player, end of episode,
// title page and trailers. Settings › About › What's new reopens it.
//
// The four previews are small drawings of those screens, built from the same
// pieces (package artwork and logos, the player's icons, pills and rings), so
// they read as the real screen at the card's size. No video, no network.
static void visualB112(GfxRect p, float t, float a, int c);
static void visualB112a(GfxRect p, float t, float a) { visualB112(p, t, a, 0); }
static void visualB112b(GfxRect p, float t, float a) { visualB112(p, t, a, 1); }
static void visualB112c(GfxRect p, float t, float a) { visualB112(p, t, a, 2); }
static void visualB112d(GfxRect p, float t, float a) { visualB112(p, t, a, 3); }
static void visualB112e(GfxRect p, float t, float a) { visualB112(p, t, a, 4); }
static void visualB112f(GfxRect p, float t, float a) { visualB112(p, t, a, 5); }
static void visualB112g(GfxRect p, float t, float a) { visualB112(p, t, a, 6); }
static const NovItem ITENS_B112[] = {
  { .grupo = "Destaques", .icone = "aj_monitor-play", .nome = "Novo player",
    .frase = "O título fica à esquerda e os botões à direita, sobre a barra, com o tempo restante e o relógio sempre à vista. Legendas e áudio abrem num botão só.", .linhas = 6 },
  { .icone = "aj_play", .nome = "Fim do episódio",
    .frase = "O próximo episódio ocupa a tela enquanto os créditos seguem num canto, com contagem e estrelas para dar nota.", .linhas = 5 },
  { .icone = "aj_layout-dashboard", .nome = "Nova página do título",
    .frase = "Logo, detalhes e sinopse primeiro, com Reproduzir embaixo, o elenco em fotos redondas e os extras em Mais sobre este título.", .linhas = 5 },
  { .icone = "aj_film", .nome = "Trailers de volta",
    .frase = "Os trailers voltam a tocar e escolhem o trailer oficial, não comerciais nem prévias. No cartão, o texto sai para o trailer aparecer.", .linhas = 5 },
  { .grupo = "Mais novidades", .icone = "aj_sparkles", .nome = "Nova abertura",
    .frase = "O logo do Nuvio+ aparece entre duas luzes que se cruzam. Escolha outra abertura em Aparência.", .linhas = 4 },
  { .icone = "aj_info", .nome = "Detalhes no player",
    .frase = "Aperte Cima nos botões para ver os detalhes do título, o formato e Começar do início.", .linhas = 4 },
  { .icone = "aj_user-round", .nome = "Editar perfis na TV",
    .frase = "Mude nome, avatar, cor, fundo e PIN em Quem está assistindo; sincroniza com o Nuvio.", .linhas = 4 },
  { .icone = "aj_palette", .nome = "Sua cor de destaque",
    .frase = "Escolha as suas cores ou um degradê nos Ajustes; sincroniza com o Nuvio.", .linhas = 4 },
  { .icone = "aj_clapperboard", .nome = "Fim do filme",
    .frase = "As recomendações mostram o logo do título, e a pergunta da nota fica embaixo do vídeo.", .linhas = 4 },
  { .icone = "aj_search", .nome = "Prévia ao avançar",
    .frase = "O vídeo continua no quadro que a miniatura do Seekr mostrou.", .linhas = 4 },
  { .grupo = "Correções", .icone = "aj_captions", .nome = "Legendas",
    .frase = "Não cobrem mais a tela de pausa e ficam dentro da janela pequena no fim do episódio ou do filme.", .linhas = 4 },
  { .icone = "aj_captions", .nome = "Legendas em sincronia",
    .frase = "As legendas não aparecem mais antes da fala nas TVs Samsung.", .linhas = 3, .plataformas = NOV_TPK | NOV_OUTRAS },
  { .icone = "aj_search", .nome = "Busca",
    .frase = "O pôster em foco cresce para baixo e não cobre mais o nome da fileira.", .linhas = 4 },
  { .icone = "aj_type", .nome = "Mais fácil de ler",
    .frase = "Fontes e Episódios no player ficam mais legíveis, e as datas em inglês seguem o formato britânico.", .linhas = 4 },
};
static const NovCena CENAS_B112[] = {
  { "Destaques", "Novo player", visualB112a, 6.0f, 0 },
  { "Destaques", "Fim do episódio", visualB112b, 6.0f, 0 },
  { "Destaques", "Nova página do título", visualB112c, 6.0f, 0 },
  { "Destaques", "Trailers de volta", visualB112d, 6.0f, 0 },
  { "Mais novidades", "Nova abertura", visualB112e, 5.0f, 0 },
  { "Mais novidades", "Editar perfis na TV", visualB112f, 6.0f, 0 },
  { "Correções", "Busca", visualB112g, 5.0f, 0 },
};
static const int ARTES_B112[] = { 4, 9, 2, 0 };
static const NovConteudo NOV_ATUAL = {
  .versao = "Nuvio+ 1.0.12", .arquivo = "novidades-better-112.txt",
  .subtitulo = NULL,
  .itens = ITENS_B112, .nItens = (int)(sizeof ITENS_B112 / sizeof *ITENS_B112),
  .cenas = CENAS_B112, .nCenas = 7,
  .artes = ARTES_B112, .nArtes = 4,
};

// A package poster (2:3) in `r`.
static void novPosterB112(int n, GfxRect r, float raio, float a) {
  char cam[600];
  GLuint t;
  snprintf(cam, sizeof cam, "%s/poster/%02d.jpg", dirArte, n);
  t = a > 0.004f ? tex_obter(cam) : 0;
  if (!t) { gfx_cor(r, raio / r.h, .2f, .21f, .24f, a); return; }
  gfx_tex_aspect_atual = tex_aspecto(cam);
  gfx_card_forcar_cover_atual = 1.0f;
  gfx_rect(r, t, GFX_CARD, 0, 0, 0, raio / r.h, 1, 1, 1, a);
  gfx_card_forcar_cover_atual = 0.0f;
  gfx_tex_aspect_atual = 0.0f;
}
// The package logo of artwork `n`, its base at (x, base), at most w x h.
static GfxRect novLogoB112(int n, float x, float base, float w, float h, float a) {
  char cam[600];
  GLuint t;
  float ap;
  snprintf(cam, sizeof cam, "%s/logo/%02d.png", dirArte, n);
  t = a > 0.004f ? tex_obter(cam) : 0;
  ap = t ? tex_aspecto(cam) : 0.0f;
  if (!t || ap <= 0.0f) return (GfxRect){ x, base, 0, 0 };
  if (w / ap > h) w = h * ap;
  h = w / ap;
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect((GfxRect){ x, base - h, w, h }, t, tex_marca_escura(cam) ? GFX_MARCA : GFX_TEXTO,
           0, 0, 0, 0, 1, 1, 1, a);
  return (GfxRect){ x, base - h, w, h };
}
// A text line of `w` (the placeholder the other cards use for prose).
static void linhaB112(float x, float y, float w, float a) {
  gfx_cor((GfxRect){ x, y, w, 7 }, .5f, 1, 1, 1, .42f * a);
}
static void circuloB112(GfxRect r, const char *icone, int foco, float a) {
  if (foco) gfx_cor(r, .5f, 1, 1, 1, .95f * a);
  else gfx_cor(r, .5f, 1, 1, 1, .16f * a);
  { float k = foco ? .1f : 1.0f, s = r.w * .46f;
    gfx_icone((GfxRect){ r.x + (r.w - s) * .5f, r.y + (r.h - s) * .5f, s, s }, icone, k, k, k + .02f, a); }
}

static void visualB112Mais(GfxRect p, float t, float a, int c);
static void visualB112(GfxRect p, float t, float a, int c) {
  if (c >= 4) { visualB112Mais(p, t, a, c); return; }
  int red = ajustes_animacoes_reduzidas();
  float e = red ? 1 : sai3(t / .45f);
  float x = p.x + 36, w = p.w - 72;
  novArte(ARTES_B112[c], p, PV_RAIO, a);
  if (c == 0) {
    // NEW PLAYER: logo on the left, the buttons on the right above the bar,
    // the clock island top right. The bar moves while the scene plays.
    float pr = .34f + .12f * (red ? .5f : fminf(1, t / 6.0f));
    float by = p.y + p.h - 196, bw = w;
    gfx_veu_css((GfxRect){ p.x, p.y + p.h * .45f, p.w, p.h * .55f }, 0, 1.0f, 1.0f, .85f * a);
    { GfxRect ilha = { p.x + p.w - 36 - 112, p.y + 32, 112, 46 };
      plrui_material(ilha, 23, 0, a * e);
      { TxtLinha l = txt_linha(TXT_G21B, "20:19", 245, 245, 245, 255);
        txt_desenhar_alpha(l, ilha.x + (ilha.w - l.w) * .5f, ilha.y + (ilha.h - l.h) * .5f, a * e); } }
    novLogoB112(ARTES_B112[c], x, by - 70 + (1 - e) * 12, w * .48f, 90, a);
    { const char *ic[3] = { "pl_captions", "pl_info", "pl_list-video" };
      for (int j = 0; j < 3; j++)
        circuloB112((GfxRect){ x + bw - (3 - j) * 66 + 6, by - 120 + (1 - e) * 12, 56, 56 }, ic[j], j == 0, a); }
    gfx_cor((GfxRect){ x, by, bw, 8 }, .5f, 1, 1, 1, .25f * a);
    gfx_cor((GfxRect){ x, by, bw * pr, 8 }, .5f, 1, 1, 1, .95f * a);
    gfx_cor((GfxRect){ x + bw * pr - 9, by - 5, 18, 18 }, .5f, 1, 1, 1, a);
    { TxtLinha l1 = txt_linha(TXT_G18M, "0:41:07", 245, 245, 245, 230);
      TxtLinha l2 = txt_linha(TXT_G18M, "\xe2\x88\x92" "1:13:52", 245, 245, 245, 200);
      txt_desenhar_alpha(l1, x, by + 22, a);
      txt_desenhar_alpha(l2, x + bw - l2.w, by + 22, a); }
  } else if (c == 1) {
    // END OF EPISODE: the next episode fills the panel, the credits recede
    // into the corner, Next episode empties its ring, stars rate the last one.
    float resta = red ? .6f : fmaxf(0, 1 - t / 6.0f);
    GfxRect jan = { p.x + p.w - 36 - p.w * .46f, p.y + 36 + (1 - e) * 10, p.w * .46f, p.w * .46f * 9 / 16 };
    gfx_veu_css((GfxRect){ p.x, p.y + p.h * .40f, p.w, p.h * .60f }, 0, 1.0f, 1.0f, .9f * a);
    novArte(4, jan, 14, a * e);
    gfx_anel(jan, 14 / jan.h, 2.0f, 1, 1, 1, .2f * a);
    { TxtLinha s = txt_linha(TXT_G18M, "Aaah! Nobody move.", 255, 255, 255, 255);
      float k = .62f;
      gfx_transformar(jan.x + jan.w * .5f, jan.y + jan.h, k, 0, 0);
      txt_desenhar_alpha(s, jan.x + (jan.w - s.w) * .5f, jan.y + jan.h - s.h - 14, a * e);
      gfx_sem_transformar(); }
    { float by = p.y + p.h - 214;
      GfxRect lg = novLogoB112(ARTES_B112[c], x, by - 140, w * .55f, 80, a * e);
      TxtLinha ep = txt_linha(TXT_G21B, "S1, E4", 240, 240, 240, 255);
      (void)lg;
      txt_desenhar_alpha(ep, x, by - 120, a * e);
      for (int j = 0; j < 2; j++) linhaB112(x, by - 76 + j * 18, w * (j ? .5f : .78f), a * e);
      { TxtLinha l = txt_linha(TXT_G21B, i18n("Próximo episódio"), 30, 30, 34, 255);
        GfxRect b = { x, by, 34 + 50 + l.w + 26, 62 };
        GfxRect an = { b.x + 22, b.y + (b.h - 32) * .5f, 32, 32 };
        gfx_cor(b, .5f, 1, 1, 1, .96f * a);
        gfx_anel(an, .5f, 2.5f, .12f, .12f, .14f, .22f * a);
        if (resta > 0) gfx_arco(an, 2.5f, 1 - resta, resta, .12f, .12f, .14f, a);
        gfx_icone((GfxRect){ an.x + 9, an.y + 9, 14, 14 }, "pl_play-f", .12f, .12f, .14f, a);
        txt_desenhar_alpha(l, an.x + 32 + 14, b.y + (b.h - l.h) * .5f, a);
        for (int j = 0; j < 5; j++)
          gfx_icone((GfxRect){ b.x + b.w + 26 + j * 38, b.y + 16, 30, 30 }, j < 4 ? "pl_star-f" : "pl_star",
                    j < 4 ? 1 : .9f, j < 4 ? .78f : .9f, j < 4 ? .25f : .9f, a); } }
  } else if (c == 2) {
    // TITLE PAGE: logo, the meta line and synopsis, then Play and the round
    // buttons; the round cast below, over the blurred artwork.
    float y = p.y + p.h * .18f + (1 - e) * 14;
    gfx_veu_css((GfxRect){ p.x, p.y + p.h * .2f, p.w, p.h * .8f }, 0, 1.0f, 1.0f, .92f * a);
    novLogoB112(ARTES_B112[c], x, y + 70, w * .62f, 80, a);
    { TxtLinha m = txt_linha(TXT_G18M, "Drama  \xc2\xb7  Mystery  \xc2\xb7  2006  \xc2\xb7  2h 10m", 225, 225, 225, 255);
      txt_desenhar_alpha(m, x, y + 96, a * e); }
    for (int j = 0; j < 3; j++) linhaB112(x, y + 140 + j * 22, w * (j == 2 ? .55f : .92f), a * e);
    { TxtLinha l = txt_linha(TXT_G21B, i18n("Reproduzir"), 24, 24, 28, 255);
      GfxRect b = { x, y + 222, 26 + 24 + 12 + l.w + 30, 60 };
      gfx_cor(b, .5f, 1, 1, 1, .96f * a);
      gfx_icone((GfxRect){ b.x + 26, b.y + 18, 24, 24 }, "pl_play-f", .1f, .1f, .12f, a);
      txt_desenhar_alpha(l, b.x + 26 + 24 + 12, b.y + (b.h - l.h) * .5f, a);
      circuloB112((GfxRect){ b.x + b.w + 18, b.y + 2, 56, 56 }, "pl_bookmark", 0, a);
      circuloB112((GfxRect){ b.x + b.w + 18 + 70, b.y + 2, 56, 56 }, "pl_info", 0, a); }
    for (int j = 0; j < 4; j++) {
      float d = 104, gx = x + j * (d + 34);
      novArte(ARTES_B112[(j + 1) % 4], (GfxRect){ gx, y + 340, d, d }, d * .5f, a * e);
      linhaB112(gx + 8, y + 340 + d + 16, d - 16, a * e);
    }
  } else {
    // TRAILERS: the card plays its trailer; the description fades and the
    // logo settles above the buttons, then the cycle repeats.
    float q = red ? 1 : anim_suave(anim_clamp((t - 1.2f) / 1.2f, 0, 1));
    float base = p.y + p.h - 186;
    gfx_veu_css((GfxRect){ p.x, p.y + p.h * .45f, p.w, p.h * .55f }, 0, 1.0f, 1.0f, (.9f - .45f * q) * a);
    { GfxRect chip = { x, p.y + 36, 150, 44 };
      plrui_material(chip, 22, 0, a * e);
      gfx_icone((GfxRect){ chip.x + 16, chip.y + 12, 20, 20 }, "pl_play-f", 1, 1, 1, a * e);
      { TxtLinha l = txt_linha(TXT_G18M, "Trailer", 245, 245, 245, 255);
        txt_desenhar_alpha(l, chip.x + 46, chip.y + (chip.h - l.h) * .5f, a * e); } }
    { float lb = anim_mistura(base - 66 - 150, base - 66 - 26, q);
      novLogoB112(ARTES_B112[c], x, lb, w * .6f * anim_mistura(1, .8f, q), 96 * anim_mistura(1, .8f, q), a); }
    { float at = a * (1 - q);
      TxtLinha m = txt_linha(TXT_G18M, "Action  \xc2\xb7  Adventure  \xc2\xb7  2024", 225, 225, 225, 255);
      txt_desenhar_alpha(m, x, base - 66 - 136, at);
      for (int j = 0; j < 2; j++) linhaB112(x, base - 66 - 96 + j * 20, w * (j ? .6f : .9f), at); }
    { TxtLinha l = txt_linha(TXT_G21B, i18n("Reproduzir"), 24, 24, 28, 255);
      GfxRect b = { x, base - 60, 26 + 24 + 12 + l.w + 30, 60 };
      gfx_cor(b, .5f, 1, 1, 1, .96f * a);
      gfx_icone((GfxRect){ b.x + 26, b.y + 18, 24, 24 }, "pl_play-f", .1f, .1f, .12f, a);
      txt_desenhar_alpha(l, b.x + 26 + 24 + 12, b.y + (b.h - l.h) * .5f, a);
      circuloB112((GfxRect){ b.x + b.w + 18, b.y + 2, 56, 56 }, "pl_bookmark", 0, a); }
  }
}

// The scenes after the four highlights (c = 4..6), drawn over a dark panel.
static void visualB112Mais(GfxRect p, float t, float a, int c) {
  int red = ajustes_animacoes_reduzidas();
  float e = red ? 1 : sai3(t / .45f);
  float x = p.x + 36, w = p.w - 72;
  if (c == 4) {
    // NEW OPENING: the real reveal (abertura.c), scaled into the panel.
    gfx_cor(p, PV_RAIO / p.h, .010f, .010f, .016f, a);   // AB_FUNDO_CINEMA
    abertura_cinema_previa((GfxRect){ p.x, p.y - 60, p.w, p.h }, red ? 3.0f : fmodf(t, 4.2f), a);
  } else if (c == 5) {
    // PROFILE EDITOR: Who's watching, Edit on the focused profile, then the
    // editor's name, colours and PIN beside the big avatar.
    float y = p.y + 70 + (1 - e) * 12, d = 112;
    novArte(ARTES_B112[2], p, PV_RAIO, .35f * a);
    gfx_rect(p, 0, GFX_VEU_CARD, 0, 0, 0, PV_RAIO / p.h, .025f, .03f, .045f, a);
    { TxtLinha l = txt_linha(TXT_G26B, i18n("Quem está assistindo?"), 245, 245, 245, 255);
      txt_desenhar_alpha(l, p.x + (p.w - l.w) * .5f, y, a * e); }
    for (int j = 0; j < 3; j++) {
      float cx = p.x + p.w * .5f + (j - 1) * (d + 46);
      GfxRect av = { cx - d * .5f, y + 70, d, d };
      novArte(ARTES_B112[j], av, d * .5f, a * e);
      if (j == 1) gfx_anel_fora(av, .5f, 6, 4, 1, 1, 1, a * e);
    }
    { TxtLinha l = txt_linha(TXT_G18M, i18n("Editar"), 30, 30, 34, 255);
      GfxRect b = { p.x + p.w * .5f - (l.w + 70) * .5f, y + 70 + d + 26, l.w + 70, 46 };
      gfx_cor(b, .5f, 1, 1, 1, .96f * a * e);
      gfx_icone((GfxRect){ b.x + 18, b.y + 13, 20, 20 }, "aj_pencil", .12f, .12f, .14f, a * e);
      txt_desenhar_alpha(l, b.x + 46, b.y + (b.h - l.h) * .5f, a * e); }
    { float ey = y + 300, k = red ? 1 : sai3((t - .9f) / .5f);
      GfxRect fol = { x, ey + (1 - k) * 20, w, 330 };
      if (k > .004f) {
        gfx_vidro_folha(fol, 22 / fol.h, a * k);
        novArte(ARTES_B112[1], (GfxRect){ fol.x + 24, fol.y + 28, 120, 120 }, 60, a * k);
        gfx_cor((GfxRect){ fol.x + 100, fol.y + 106, 40, 40 }, .5f, 1, 1, 1, .95f * a * k);
        gfx_icone((GfxRect){ fol.x + 109, fol.y + 115, 22, 22 }, "aj_pencil", .1f, .1f, .12f, a * k);
        { TxtLinha n = txt_linha(TXT_G26B, "Sam", 245, 245, 245, 255);
          txt_desenhar_alpha(n, fol.x + 170, fol.y + 50, a * k); }
        linhaB112(fol.x + 170, fol.y + 106, w * .4f, a * k);
        { static const float cor[6][3] = { {.93f,.33f,.33f}, {.96f,.62f,.2f}, {.95f,.85f,.3f},
                                           {.35f,.8f,.5f}, {.3f,.6f,.95f}, {.65f,.45f,.95f} };
          for (int j = 0; j < 6; j++) {
            GfxRect sw = { fol.x + 24 + j * 64, fol.y + 180, 46, 46 };
            gfx_cor(sw, .5f, cor[j][0], cor[j][1], cor[j][2], a * k);
            if (j == 4) gfx_anel_fora(sw, .5f, 4, 3, 1, 1, 1, a * k);
          } }
        for (int j = 0; j < 4; j++)
          gfx_cor((GfxRect){ fol.x + 24 + j * 40, fol.y + 268, 26, 26 }, .5f, 1, 1, 1, .85f * a * k);
      } }
  } else {
    // SEARCH: a row of posters; the focused one grows downward from the top,
    // and the row's name stays clear above it.
    float y = p.y + 200 + (1 - e) * 12, pw = 150, ph = pw * 1.5f, g = 22;
    float z = red ? 1 : anim_suave(anim_clamp((t - .6f) / .6f, 0, 1));
    novArte(ARTES_B112[3], p, PV_RAIO, .3f * a);
    gfx_rect(p, 0, GFX_VEU_CARD, 0, 0, 0, PV_RAIO / p.h, .025f, .03f, .045f, a);
    { GfxRect campo = { x, p.y + 60, w, 64 };
      plrui_material(campo, 32, 0, a * e);
      gfx_icone((GfxRect){ campo.x + 22, campo.y + 20, 24, 24 }, "aj_search", .85f, .88f, .92f, a * e);
      { TxtLinha l = txt_linha(TXT_G21B, "prest", 245, 245, 245, 255);
        txt_desenhar_alpha(l, campo.x + 62, campo.y + (campo.h - l.h) * .5f, a * e); } }
    { TxtLinha l = txt_linha(TXT_G21B, i18n("Filmes"), 245, 245, 245, 255);
      txt_desenhar_alpha(l, x, y - l.h - 14, a); }
    for (int j = 0; j < 4; j++) {
      float s = j == 1 ? 1.0f + .14f * z : 1.0f;
      float px = x + j * (pw + g) + (j > 1 ? pw * .14f * z : 0) - (j == 1 ? 0 : 0);
      GfxRect r = { px, y, pw * s, ph * s };
      novPosterB112((j + 2) % 12, r, 12, a * e);
      if (j == 1) gfx_anel_fora(r, 12 / r.h, 5, 3, 1, 1, 1, a * z);
    }
  }
}
