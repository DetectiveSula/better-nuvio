// Better Nuvio 1.0.2 (12 Nov 2026): the first What's new card of Better Nuvio
// itself. It replaces Nuvio Legacy's 2.0.8 card and also covers 1.0.1, which
// shipped without one. Settings › About › What's new reopens it.
static void visualB102(GfxRect p, float t, float a, int c);
static void visualB102a(GfxRect p, float t, float a) { visualB102(p, t, a, 0); }
static void visualB102b(GfxRect p, float t, float a) { visualB102(p, t, a, 1); }
static void visualB102c(GfxRect p, float t, float a) { visualB102(p, t, a, 2); }
static const NovItem ITENS_B102[] = {
  { .grupo = "Destaques", .icone = "aj_rotate-cw", .nome = "Ajustes em todas as TVs",
    .frase = "Ajustes, layout da Home e preferências pessoais seguem a sua conta entre as TVs e voltam depois de reinstalar.", .linhas = 5 },
  { .icone = "aj_zap", .nome = "TorBox Instant",
    .frase = "Os links do AIOStreams voltam a tocar, a conferência da fonte ficou bem mais rápida e torrents fora da sua conta não gastam mais as tentativas.", .linhas = 6 },
  { .icone = "aj_book-open", .nome = "Guia de uso",
    .frase = "Atualizado para o Better Nuvio: ajustes na conta, TorBox, depois do filme, pular abertura e mais.", .linhas = 5 },
  { .grupo = "Mais novidades", .icone = "aj_calendar", .nome = "Agenda",
    .frase = "A data do próximo episódio agora é a mesma da lista de episódios.", .linhas = 4 },
  { .icone = "aj_palette", .nome = "Aparência mantida",
    .frase = "Numa instalação nova, a segunda abertura não volta mais o tema, a cor da logo e a profundidade ao padrão.", .linhas = 5 },
  { .icone = "aj_key-round", .nome = "Adições em cache do TorBox",
    .frase = "Ligue em Integrações para tocar na hora os torrents em cache que ainda não estão na sua conta TorBox.", .linhas = 5 },
};
static const NovCena CENAS_B102[] = {
  { "Destaques", "Ajustes em todas as TVs", visualB102a, 5.5f, 0 },
  { "Destaques", "TorBox Instant", visualB102b, 5.5f, 0 },
  { "Destaques", "Guia de uso", visualB102c, 5.5f, 0 },
};
static const int ARTES_B102[] = { 2, 4, 7 };
static const NovConteudo NOV_ATUAL = {
  .versao = "Better Nuvio 1.0.2", .arquivo = "novidades-better-102.txt",
  .subtitulo = NULL,
  .itens = ITENS_B102, .nItens = (int)(sizeof ITENS_B102 / sizeof *ITENS_B102),
  .cenas = CENAS_B102, .nCenas = 3,
  .artes = ARTES_B102, .nArtes = 3,
};

// Small, static feature mockups (same pieces as the 2.0.8 card). Only their
// entrance moves; no live video, no network.
static void visualB102(GfxRect p, float t, float a, int c) {
  float e = ajustes_animacoes_reduzidas() ? 1 : sai3(t / .45f);
  float x = p.x + 40, y = p.y + 100 + (1 - e) * 16;
  float w = p.w - 80;
  const char *icons[] = { "aj_rotate-cw", "aj_zap", "aj_book-open" };
  novArte(ARTES_B102[c], (GfxRect){p.x, p.y, p.w, p.h}, PV_RAIO, .68f * a);
  gfx_rect(p, 0, GFX_VEU_CARD, 0, 0, 0, PV_RAIO / p.h, .025f, .03f, .045f, a);
  float vs = fminf(1, (p.h - 230) / 540);
  gfx_transformar(p.x, p.y, vs, 0, 0);
  gfx_icone((GfxRect){x, p.y + 36, 40, 40}, icons[c], .85f, .90f, 1, a);
  if (c == 0) {
    // Two TVs showing the same settings, linked through the account.
    float tw = w * .40f;
    for (int j = 0; j < 2; j++) {
      GfxRect tv = {x + j * (w - tw), y + 20, tw, tw * .62f};
      novArte(ARTES_B102[j], tv, 12, a * e);
      for (int k = 0; k < 3; k++)
        gfx_cor((GfxRect){tv.x + 14, tv.y + tv.h + 22 + k * 16, tv.w * (.8f - k * .15f), 5}, .5f, 1, 1, 1, .32f * a);
    }
    gfx_icone((GfxRect){x + w / 2 - 28, y + 20 + tw * .31f - 28, 56, 56}, "aj_rotate-cw", .5f, .9f, .9f, a * e);
  } else if (c == 1) {
    // "Checking the source": the old wait against the new one.
    for (int j = 0; j < 2; j++) {
      float yy = y + 40 + j * 120;
      gfx_vidro_folha((GfxRect){x, yy, w, 84}, 18 / 84.0f, a * e);
      gfx_icone((GfxRect){x + 24, yy + 22, 40, 40}, j ? "aj_zap" : "aj_clock", .9f, .94f, 1, a);
      gfx_cor((GfxRect){x + 88, yy + 38, (w - 130) * (j ? .2f : 1.0f), 8}, .5f, j ? .55f : .95f, j ? .92f : .45f, j ? .7f : .4f, .85f * a);
    }
  } else {
    // The guide: chapters on the left, entries on the right.
    for (int j = 0; j < 4; j++) {
      GfxRect r = {x, y + j * 84, w * .34f, 66};
      gfx_vidro_folha(r, 16 / r.h, a * e * (j == 0 ? 1 : .7f));
      gfx_cor((GfxRect){r.x + 18, r.y + 30, r.w - 40, 5}, .5f, 1, 1, 1, .45f * a);
    }
    for (int j = 0; j < 3; j++) {
      GfxRect r = {x + w * .38f, y + j * 112, w * .62f, 92};
      gfx_vidro_folha(r, 18 / r.h, a * e);
      gfx_icone((GfxRect){r.x + 20, r.y + 26, 40, 40}, icons[j], .9f, .94f, 1, a);
      gfx_cor((GfxRect){r.x + 80, r.y + 40, r.w - 110, 6}, .5f, 1, 1, 1, .5f * a);
    }
  }
  gfx_sem_transformar();
}
