// PAINEL DE 30 DIAS (A + C) — o resumo do Perfil e Stats.
// Desenho: docs/profile-stats-dashboard/PLAN.md e o artboard "Option A + C" do
// canvas de mockups. Dado: PerfilDados, bloco "PAINEL DE 30 DIAS", montado por
// perfilnuvio.c a partir de Nuvio sync.
//
// UMA TELA, SEM ROLAGEM na TV (a regra desta tela continua): cabecalho com a
// pilula Amigos, cinco numeros, o grafico dia a dia ao lado do relogio de 24 h,
// e embaixo os cartazes mais vistos, o mix e quatro fatos. TRES paradas de
// foco: o grafico (dia a dia), os cartazes e a pilula Amigos. OK num dia abre
// a folha do dia; OK num cartaz abre o titulo; OK na pilula abre o duelo.
//
// Incluido por perfil.c: usa os estaticos de la (dados, secao, item, dia...).

// Sem a pilula da barra, 48 (o mockup); com ela, o mesmo 20 abaixo dela que
// o resto da tela usa (PF_TOPO = 56 + PF_DY).
#define PN_TOPO      (PF_DY > 0.0f ? PF_TOPO : 48.0f)
#define PN_AV         84.0f
#define PN_GAP        18.0f
#define PN_KPI_Y     (PN_TOPO + PN_AV + 22.0f)
#define PN_KPI_H     128.0f
#define PN_MEIO_Y    (PN_KPI_Y + PN_KPI_H + PN_GAP)
#define PN_BAR_GAP     6.0f
#define PN_FAIXA_H    64.0f
#define PN_ROSTO      40.0f
#define PN_ROSTOS      3

// A cor do FILME no grafico e no mix. Segunda cor de DADO, nunca de foco (D2).
#define PN_FILME_R 0.949f
#define PN_FILME_G 0.702f
#define PN_FILME_B 0.353f

static float pnMeioH(void) { float h = 372.0f - PF_DY; return h < 300.0f ? 300.0f : h; }
static float pnBaixoY(void) { return PN_MEIO_Y + pnMeioH() + PN_GAP; }

static int nTopo(void) { return limitar(dados.nTopo, 0, PERFIL_MAX_TOPO); }
static const PerfilObra *obraDe(int i) {
  return i >= 0 && i < dados.nObras && i < PERFIL_MAX_OBRAS ? &dados.obras[i] : NULL;
}
static const char *tituloObra(const PerfilObra *o) {
  return !o ? "—" : o->titulo[0] ? o->titulo : o->id;
}

// --- geometria -----------------------------------------------------------------
// RETRATO (toque, tela em pe): tudo em coluna, rolando por pfDeslocamento.
static float pnRetratoY(int bloco) {
  // 0 numeros (3 linhas de 2), 1 grafico, 2 relogio, 3 cartazes, 4 mix, 5 fatos.
  static const float H[6] = { 3 * PN_KPI_H + 2 * PN_GAP, 372.0f, 372.0f, 330.0f, 300.0f, 0 };
  float y = PN_KPI_Y - pfDeslocamento();
  for (int i = 0; i < bloco; i++) y += H[i] + PN_GAP;
  return y;
}
static GfxRect rKpi(int i) {
  if (pfRetrato()) {
    float w = (PF_W - PN_GAP) * 0.5f;
    return (GfxRect){ PF_X + (w + PN_GAP) * (float)(i % 2),
                      pnRetratoY(0) + (PN_KPI_H + PN_GAP) * (float)(i / 2), w, PN_KPI_H };
  }
  { float w = (PF_W - PN_GAP * 4.0f) / 5.0f;
    return (GfxRect){ PF_X + (w + PN_GAP) * (float)i, PN_KPI_Y, w, PN_KPI_H }; }
}
static float pnGrafW(void) { return floorf((PF_W - PN_GAP) * 0.675f); }
static GfxRect rGrafico(void) {
  if (pfRetrato()) return (GfxRect){ PF_X, pnRetratoY(1), PF_W, 372.0f };
  return (GfxRect){ PF_X, PN_MEIO_Y, pnGrafW(), pnMeioH() };
}
static GfxRect rRelogio(void) {
  if (pfRetrato()) return (GfxRect){ PF_X, pnRetratoY(2), PF_W, 372.0f };
  return (GfxRect){ PF_X + pnGrafW() + PN_GAP, PN_MEIO_Y, PF_W - pnGrafW() - PN_GAP, pnMeioH() };
}
static float pnTopoW(void) { return floorf((PF_W - 2.0f * PN_GAP) * 0.49f); }
static float pnMixW(void)  { return floorf((PF_W - 2.0f * PN_GAP) * 0.22f); }
static GfxRect rTopo(void) {
  if (pfRetrato()) return (GfxRect){ PF_X, pnRetratoY(3), PF_W, 330.0f };
  return (GfxRect){ PF_X, pnBaixoY(), pnTopoW(), PF_CONTEUDO_H - pnBaixoY() };
}
static GfxRect rMix(void) {
  if (pfRetrato()) return (GfxRect){ PF_X, pnRetratoY(4), PF_W, 300.0f };
  return (GfxRect){ PF_X + pnTopoW() + PN_GAP, pnBaixoY(), pnMixW(), PF_CONTEUDO_H - pnBaixoY() };
}
static GfxRect rFato(int i) {
  if (pfRetrato()) {
    float w = (PF_W - PN_GAP) * 0.5f;
    return (GfxRect){ PF_X + (w + PN_GAP) * (float)(i % 2),
                      pnRetratoY(5) + (140.0f + PN_GAP) * (float)(i / 2), w, 140.0f };
  }
  { float x0 = PF_X + pnTopoW() + pnMixW() + 2.0f * PN_GAP, w0 = PF_X + PF_W - x0;
    float w = (w0 - PN_GAP) * 0.5f, h = (PF_CONTEUDO_H - pnBaixoY() - PN_GAP) * 0.5f;
    return (GfxRect){ x0 + (w + PN_GAP) * (float)(i % 2), pnBaixoY() + (h + PN_GAP) * (float)(i / 2), w, h }; }
}
// A PILULA AMIGOS, a direita do cabecalho: ate tres rostos, "Amigos" e quantos.
static int rostosCabem(void) {
  int n = socialvis_n_amigos();
  return n < PN_ROSTOS ? n : PN_ROSTOS;
}
static GfxRect rPilula(void) {
  int m = rostosCabem();
  float w = 28.0f + (float)m * (PN_ROSTO - 10.0f) + 10.0f + 16.0f
          + (float)txt_largura(TXT_ROW_TITULO, "Amigos") * PF_ESC_CARD + 72.0f;
  return (GfxRect){ PF_X + PF_W - w, PN_TOPO + (PN_AV - 64.0f) * 0.5f, w, 64.0f };
}

// --- utilidades de texto -------------------------------------------------------
// Uma linha reduzida (estilo-base em `esc`) e a largura dela, sem desenhar.
static float pnLarg(TxtEstilo e, const char *s, float esc) { return (float)txt_largura(e, s) * esc; }
static void pnCentro(TxtEstilo e, const char *s, int c, float cx, float y, float esc, float a) {
  TxtLinha l = txt_linha(e, s, c, c, c, 255);
  txtEsc(l, cx - (float)l.w * esc * 0.5f, y, esc, a);
}
// A data local do dia `i` da janela (0 = 29 dias atras).
static struct tm pnDia(int i) {
  time_t t0 = (time_t)(dados.janelaInicioMs / 1000);
  struct tm tm;
  localtime_r(&t0, &tm);
  tm.tm_mday += i; tm.tm_hour = 12; tm.tm_isdst = -1;
  t0 = mktime(&tm);
  localtime_r(&t0, &tm);
  return tm;
}
static const char *const PN_MESES[12] = { "Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho",
                                          "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro" };
static const char *const PN_DOW[7] = { "Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb" };
static const char *const PN_DIAS[7] = { "Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado" };
static void pnData(char *s, size_t n, const struct tm *tm) {
  snprintf(s, n, "%s, %d %s", i18n(PN_DOW[tm->tm_wday]), tm->tm_mday, i18n(PN_MESES[tm->tm_mon]));
}
// "From T2 E4–E8", "From T2 E4", "Duna" (filme).
static void pnItem(char *s, size_t n, const PerfilDiaItem *it) {
  const PerfilObra *o = obraDe(it->obra);
  const char *t = o ? tituloObra(o) : "—";
  // O codigo do episodio pela chave que o app ja traduz ("T%dE%d" -> "S%dE%d").
  char ep[32];
  if (it->epIni <= 0) { snprintf(s, n, "%s", t); return; }
  snprintf(ep, sizeof ep, i18n("T%dE%d"), it->temporada, it->epIni);
  if (it->epFim > it->epIni) snprintf(s, n, "%s %s\xe2\x80\x93%d", t, ep, it->epFim);
  else snprintf(s, n, "%s %s", t, ep);
}
static int pnMaratonaDia(void) {
  if (dados.maratona.obra < 0 || dados.maratona.fimMs <= 0) return -1;
  for (int i = PERFIL_DIAS - 1; i >= 0; i--) {
    struct tm d = pnDia(i), m;
    time_t t = (time_t)(dados.maratona.fimMs / 1000);
    localtime_r(&t, &m);
    if (d.tm_year == m.tm_year && d.tm_yday == m.tm_yday) return i;
  }
  return -1;
}

// --- cabecalho -------------------------------------------------------------------
static void ponteiroPilula(int i, int b) {
  (void)i; (void)b;
  if (modo != PF_RESUMO || !temDados || rostosCabem() <= 0) return;
  secao = 2;
}
static void desenharCabecalhoPainel(Uint32 agora, float a) {
  GfxRect av = { PF_X, PN_TOPO, PN_AV, PN_AV };   // fixo: no retrato a coluna rola por baixo
  float tx = PF_X + PN_AV + 24.0f, cy = av.y + PN_AV * 0.5f;
  float wTxt = PF_W - PN_AV - 24.0f - (rostosCabem() > 0 ? rPilula().w + 40.0f : 0.0f);
  const char *nm = dados.nome[0] ? dados.nome : dados.usuario[0] ? dados.usuario : "Perfil e Stats";
  char sub[240], d0[64], d1[64];
  struct tm t0 = pnDia(0), t1 = pnDia(PERFIL_DIAS - 1);
  avatarEu(av, 1, a);
  snprintf(d0, sizeof d0, "%d %s", t0.tm_mday, i18n(PN_MESES[t0.tm_mon]));
  snprintf(d1, sizeof d1, "%d %s", t1.tm_mday, i18n(PN_MESES[t1.tm_mon]));
  if (dados.cobertura & PF_COB_NUVIO)
    snprintf(sub, sizeof sub, "%s \xc2\xb7 %s \xe2\x80\x93 %s \xc2\xb7 Nuvio Sync", i18n("Últimos 30 dias"), d0, d1);
  else snprintf(sub, sizeof sub, "%s", dados.periodo);
  { TxtLinha n = txt_linha_corta(TXT_TITULO2, nm, 245, 245, 243, 255, wTxt / (PF_ESC_NOME * 0.9f));
    TxtLinha l2 = txt_linha_corta(TXT_CAPTION2, sub, PF_FRACO, PF_FRACO, PF_FRACO, 255, wTxt / PF_ESC_AMIGO);
    float hN = (float)n.h * PF_ESC_NOME * 0.9f, h2 = (float)l2.h * PF_ESC_AMIGO;
    float y = cy - (hN + 4.0f + h2) * 0.5f;
    txtEsc(n, tx, y, PF_ESC_NOME * 0.9f, a);
    txtEsc(l2, tx, y + hN + 4.0f, PF_ESC_AMIGO, a); }
  // A PILULA AMIGOS (D1): o caminho para o duelo, que antes era o cartao.
  if (rostosCabem() > 0) {
    GfxRect p = rPilula();
    float f = focoAmigos, x = p.x + 18.0f, ar, ag, ab;
    char cont[16];
    acento(&ar, &ag, &ab);
    if (ajustes_vidro()) gfx_cor(p, 0.5f, .92f, .93f, 1.0f, (.06f + .10f * f) * a);
    else gfx_cor(p, 0.5f, .082f + .09f * f, .086f + .09f * f, .102f + .1f * f, a);
    if (f > 0.01f) gfx_anel_fora(p, 0.5f, 4.0f, 3.0f, ar, ag, ab, f * a);
    pfAlvoCorpo(p.x, p.y, p.w, p.h, ponteiroPilula, NULL, 0, 0);
    for (int k = rostosCabem() - 1; k >= 0; k--)
      svd_rosto((GfxRect){ x + (float)k * (PN_ROSTO - 10.0f), p.y + 12.0f, PN_ROSTO, PN_ROSTO },
                socialvis_amigo(k), 0.0f, a, agora);
    x += (float)rostosCabem() * (PN_ROSTO - 10.0f) + 10.0f + 12.0f;
    { TxtLinha l = txt_linha(TXT_ROW_TITULO, "Amigos", 245, 245, 243, 255);
      txtEsc(l, x, p.y + (p.h - (float)l.h * PF_ESC_CARD) * 0.5f, PF_ESC_CARD, a);
      x += (float)l.w * PF_ESC_CARD + 10.0f; }
    snprintf(cont, sizeof cont, "%d", socialvis_n_amigos());
    escrever(TXT_CAPTION2, cont, PF_APOIO, x, p.y + 22.0f, 60.0f, PF_ESC_AMIGO, a);
    if (socialvis_n_ao_vivo() > 0)
      gfx_cor((GfxRect){ p.x + p.w - 22.0f, p.y + 26.0f, 10.0f, 10.0f }, 0.5f, .93f, .27f, .27f, a);
  }
}

// --- numeros ---------------------------------------------------------------------
static void desenharNumerosPainel(float a0) {
  char val[5][48], sub[5][96], t[48];
  static const char *const rot[5] = { "Horas assistidas", "Títulos", "Episódios", "Filmes", "Sequência atual" };
  float ar, ag, ab;
  int nuvio = (dados.cobertura & PF_COB_NUVIO) != 0;
  acento(&ar, &ag, &ab);
  tempo(t, sizeof t, dados.minutos);
  snprintf(val[0], sizeof val[0], "%s%s", (dados.cobertura & PF_COB_ESTIMADO) ? "\xe2\x89\x88 " : "", t);
  if (dados.diasAtivosMes > 0) { tempo(t, sizeof t, dados.minutos / dados.diasAtivosMes);
                                 snprintf(sub[0], sizeof sub[0], i18n("%s por dia ativo"), t); }
  else snprintf(sub[0], sizeof sub[0], "%s", i18n("Nenhum dia ativo"));
  if (nuvio) numero(val[1], sizeof val[1], dados.titulos); else snprintf(val[1], sizeof val[1], "\xe2\x80\x94");
  snprintf(sub[1], sizeof sub[1], "%s", i18n("filmes e séries"));
  numero(val[2], sizeof val[2], dados.episodios);
  if (dados.marcados > 0) snprintf(sub[2], sizeof sub[2], i18n("+%d marcados como vistos"), dados.marcados);
  else snprintf(sub[2], sizeof sub[2], "%s", i18n("terminados"));
  numero(val[3], sizeof val[3], dados.filmes);
  tempo(t, sizeof t, dados.minutosFilmes);
  snprintf(sub[3], sizeof sub[3], i18n("%s de filme"), t);
  snprintf(val[4], sizeof val[4], dados.streakAtual == 1 ? i18n("%d dia") : i18n("%d dias"), dados.streakAtual);
  snprintf(sub[4], sizeof sub[4], i18n("%d de %d dias ativos"), dados.diasAtivosMes, PERFIL_DIAS);
  for (int i = 0; i < 5; i++) {
    GfxRect r = rKpi(i);
    float a = a0 * surge(i), w = r.w - PF_PAD_X * 2.0f;
    if (a <= 0.002f) continue;
    cartao(r, a);
    escrever(TXT_CAPTION2, rot[i], PF_MEDIO, r.x + PF_PAD_X, r.y + 18.0f, w, PF_ESC_AMIGO, a);
    { int c = i == 4;
      TxtLinha v = txt_linha_corta(TXT_TITULO3, val[i], c ? acentoI(ar) : 245, c ? acentoI(ag) : 245,
                                   c ? acentoI(ab) : 243, 255, w / PF_ESC_BIG);
      txtEsc(v, r.x + PF_PAD_X, r.y + 42.0f, PF_ESC_BIG, a); }
    escrever(TXT_CAPTION2, sub[i], PF_APOIO, r.x + PF_PAD_X, r.y + r.h - 34.0f, w, PF_ESC_APOIO, a);
  }
}

// --- dia a dia -------------------------------------------------------------------
static void ponteiroDia(int i, int b) {
  (void)b;
  if (modo != PF_RESUMO || !temDados || i < 0 || i >= PERFIL_DIAS) return;
  secao = 0; dia = i;
}
static void desenharGrafico(float a) {
  GfxRect r = rGrafico();
  float ar, ag, ab, x0 = r.x + PF_PAD_X, w = r.w - PF_PAD_X * 2.0f, y;
  int max = 60;
  acento(&ar, &ag, &ab);
  cartao(r, a);
  y = tituloCartao(r, "Dia a dia", a);
  // A legenda, a direita do titulo.
  { float lx = r.x + r.w - PF_PAD_X, ly = r.y + PF_PAD_Y + 4.0f;
    lx -= pnLarg(TXT_CAPTION2, "Filmes", PF_ESC_APOIO);
    escrever(TXT_CAPTION2, "Filmes", PF_MEDIO, lx, ly, 200.0f, PF_ESC_APOIO, a);
    lx -= 22.0f; gfx_cor((GfxRect){ lx, ly + 3.0f, 14.0f, 14.0f }, 4.0f / 14.0f, PN_FILME_R, PN_FILME_G, PN_FILME_B, a);
    lx -= 22.0f + pnLarg(TXT_CAPTION2, "Episódios", PF_ESC_APOIO);
    escrever(TXT_CAPTION2, "Episódios", PF_MEDIO, lx, ly, 200.0f, PF_ESC_APOIO, a);
    lx -= 22.0f; gfx_cor((GfxRect){ lx, ly + 3.0f, 14.0f, 14.0f }, 4.0f / 14.0f, ar, ag, ab, a); }
  // A FAIXA DO DIA ESCOLHIDO: cartazes pequenos, data e tempo, o que foi visto.
  { GfxRect f = { x0, y + 12.0f, w, PN_FAIXA_H };
    int d = limitar(dia, 0, PERFIL_DIAS - 1), n = limitar(dados.nDiaItens[d], 0, PERFIL_ITENS_DIA);
    struct tm tm = pnDia(d);
    char l1[200], l2[400], data[80], tt[48];
    float px = f.x + 12.0f;
    gfx_cor(f, 16.0f / f.h, 0, 0, 0, .28f * a);
    for (int k = 0; k < n && k < 2; k++) {
      const PerfilObra *o = obraDe(dados.diaItens[d][k].obra);
      GfxRect pr = { px, f.y + 8.0f, 32.0f, 48.0f };
      svd_poster(pr, o ? o->poster : "", 6.0f / pr.h, a);
      px += 40.0f;
    }
    pnData(data, sizeof data, &tm);
    tempo(tt, sizeof tt, dados.minutosDia[d]);
    snprintf(l1, sizeof l1, "%s \xc2\xb7 %s", data, dados.minutosDia[d] ? tt : i18n("nada assistido"));
    l2[0] = 0;
    for (int k = 0; k < n; k++) {
      char it[200];
      pnItem(it, sizeof it, &dados.diaItens[d][k]);
      snprintf(l2 + strlen(l2), sizeof l2 - strlen(l2), "%s%s", k ? " \xc2\xb7 " : "", it);
    }
    { float tw = f.x + f.w - px - 12.0f;
      int tag = d == pnMaratonaDia();
      if (tag) {
        float lw = pnLarg(TXT_CAPTION2, "Maior maratona", PF_ESC_APOIO) + 24.0f;
        GfxRect tg = { f.x + f.w - 12.0f - lw, f.y + (f.h - 30.0f) * 0.5f, lw, 30.0f };
        gfx_cor(tg, 0.5f, 0.60f, 0.95f, 0.92f, a);
        escrever(TXT_CAPTION2, "Maior maratona", 12, tg.x + 12.0f, tg.y + 5.0f, lw, PF_ESC_APOIO, a);
        tw -= lw + 12.0f;
      }
      escrever(TXT_ROW_TITULO, l1, PF_FORTE, px, f.y + 8.0f, tw, PF_ESC_LIN, a);
      escrever(TXT_CAPTION2, l2[0] ? l2 : i18n("Nenhum título neste dia."), PF_APOIO, px, f.y + 36.0f, tw, PF_ESC_APOIO, a); }
    y = f.y + f.h + 14.0f; }
  // AS BARRAS: episodio no acento embaixo, filme em ambar em cima.
  { float base = r.y + r.h - PF_PAD_Y - 22.0f, altura = base - y;
    float bw = (w - PN_BAR_GAP * (float)(PERFIL_DIAS - 1)) / (float)PERFIL_DIAS;
    for (int i = 0; i < PERFIL_DIAS; i++) if (dados.minutosDia[i] > max) max = dados.minutosDia[i];
    for (int i = 0; i < PERFIL_DIAS; i++) {
      float bx = x0 + (bw + PN_BAR_GAP) * (float)i;
      int md = dados.minutosDia[i], mf = dados.minutosFilmeDia[i] > md ? md : dados.minutosFilmeDia[i];
      float hT = md > 0 ? fmaxf(6.0f, altura * (float)md / (float)max) : 4.0f;
      float hF = md > 0 ? hT * (float)mf / (float)md : 0.0f, hS = hT - hF;
      float f = i == dia ? focoCal : 0.0f;
      struct tm tm = pnDia(i);
      char num[8];
      pfAlvoCorpo(bx, y, bw, base - y + 22.0f, ponteiroDia, NULL, i, 0);
      if (md <= 0) gfx_cor((GfxRect){ bx, base - 4.0f, bw, 4.0f }, 0.5f, 1, 1, 1, .08f * a);
      else {
        if (hS > 0.5f) gfx_cor((GfxRect){ bx, base - hS, bw, hS }, fminf(6.0f, hS * 0.5f) / hS, ar, ag, ab, a);
        if (hF > 0.5f) gfx_cor((GfxRect){ bx, base - hT, bw, hF - (hS > 0.5f ? 3.0f : 0.0f) },
                               fminf(6.0f, hF * 0.5f) / hF, PN_FILME_R, PN_FILME_G, PN_FILME_B, a);
      }
      if (f > 0.01f) {
        GfxRect fr = { bx, base - hT, bw, hT };
        gfx_anel_fora(fr, fminf(6.0f, hT * 0.5f) / hT, 4.0f, 3.0f, .96f, .97f, .97f, f * a);
      }
      snprintf(num, sizeof num, "%d", tm.tm_mday);
      pnCentro(TXT_CAPTION2, num, i == dia ? PF_FORTE : PF_FRACO, bx + bw * 0.5f, base + 6.0f, PF_ESC_APOIO * 0.92f, a);
    } }
}

// --- relogio de 24 h ---------------------------------------------------------------
static void desenharRelogio(float a) {
  GfxRect r = rRelogio();
  float ar, ag, ab, y;
  int max = 0, pico = -1, total = 0;
  for (int h = 0; h < 24; h++) {
    total += dados.minutosHora[h];
    if (dados.minutosHora[h] > max) { max = dados.minutosHora[h]; pico = h; }
  }
  acento(&ar, &ag, &ab);
  cartao(r, a);
  y = tituloCartao(r, "Hora do dia", a);
  { float disp = r.y + r.h - PF_PAD_Y - y - 16.0f;
    float rMax = fminf(150.0f, disp * 0.5f - 18.0f), r0 = rMax * 0.36f;
    float cx = r.x + PF_PAD_X + rMax + 18.0f, cy = y + 8.0f + disp * 0.5f;
    for (int h = 0; h < 24; h++) {
      int m = dados.minutosHora[h];
      float fr = max > 0 ? (float)m / (float)max : 0.0f;
      float r1 = r0 + 6.0f + fr * (rMax - r0 - 6.0f);
      float ini = (float)h / 24.0f + 0.12f / 24.0f, frac = 1.0f / 24.0f - 0.24f / 24.0f;
      if (m <= 0) gfx_setor(cx, cy, r0, r0 + 6.0f, ini, frac, 1, 1, 1, .08f * a);
      else if (h == pico) gfx_setor(cx, cy, r0, r1, ini, frac, ar + (1 - ar) * .55f, ag + (1 - ag) * .55f, ab + (1 - ab) * .55f, a);
      else gfx_setor(cx, cy, r0, r1, ini, frac, ar, ag, ab, (fr >= 0.45f ? 1.0f : 0.62f) * a);
    }
    { static const char *const marca[4] = { "00", "06", "12", "18" };
      for (int k = 0; k < 4; k++) {
        float px, py;
        setor_ponto(cx, cy, rMax + 14.0f, (float)k * 0.25f, &px, &py);
        pnCentro(TXT_CAPTION2, marca[k], PF_FRACO, px, py - 9.0f, PF_ESC_APOIO * 0.9f, a);
      } }
    if (pico >= 0) {
      char ph[16];
      snprintf(ph, sizeof ph, "%02d:00", pico);
      pnCentro(TXT_TITULO3, ph, 245, cx, cy - 24.0f, PF_ESC_COMUM * 0.9f, a);
      pnCentro(TXT_CAPTION2, "pico", PF_APOIO, cx, cy + 8.0f, PF_ESC_APOIO, a);
    }
    // OS QUATRO PERIODOS, a direita do relogio.
    { int faixa[4] = { 0, 0, 0, 0 };
      static const char *const rot[4] = { "Madrugada · 21h–6h", "Noite · 18h–21h", "Tarde · 12h–18h", "Manhã · 6h–12h" };
      float tx = cx + rMax + 40.0f, tw = r.x + r.w - PF_PAD_X - tx, ty = y + 18.0f;
      for (int h = 0; h < 24; h++) {
        int k = (h >= 21 || h < 6) ? 0 : h >= 18 ? 1 : h >= 12 ? 2 : 3;
        faixa[k] += dados.minutosHora[h];
      }
      if (total <= 0) {
        escrever(TXT_CAPTION2, "Sem horários nos últimos 30 dias.", PF_APOIO, tx, ty, tw, PF_ESC_AMIGO, a);
        return;
      }
      for (int k = 0; k < 4; k++) {
        char pc[16];
        snprintf(pc, sizeof pc, "%d%%", (faixa[k] * 100 + total / 2) / total);
        escrever(TXT_TITULO3, pc, 245, tx, ty, tw, PF_ESC_COMUM * 0.85f, a);
        escrever(TXT_CAPTION2, rot[k], PF_APOIO, tx, ty + 30.0f, tw, PF_ESC_APOIO, a);
        ty += (disp - 10.0f) / 4.0f;
      } } }
}

// --- cartazes ----------------------------------------------------------------------
static void ponteiroTopo(int i, int b) {
  (void)b;
  if (modo != PF_RESUMO || !temDados || i < 0 || i >= nTopo()) return;
  secao = 1; item = i;
}
static void desenharTopo(float a) {
  GfxRect r = rTopo();
  int n = nTopo();
  float y, ar, ag, ab;
  acento(&ar, &ag, &ab);
  cartao(r, a);
  y = tituloCartao(r, "Mais vistos", a) + 14.0f;
  if (!n) { escrever(TXT_CAPTION2, "Nada assistido nos últimos 30 dias.", PF_APOIO, r.x + PF_PAD_X, y + 8.0f,
                     r.w - PF_PAD_X * 2.0f, PF_ESC_AMIGO, a); return; }
  { float ph = r.y + r.h - PF_PAD_Y - y - 50.0f, pw = ph * 2.0f / 3.0f;
    float gap = 22.0f, maxW = (r.w - PF_PAD_X * 2.0f - gap * (PERFIL_MAX_TOPO - 1)) / PERFIL_MAX_TOPO;
    if (pw > maxW) { pw = maxW; ph = pw * 1.5f; }
    for (int i = 0; i < n; i++) {
      const PerfilObra *o = obraDe(dados.topo[i]);
      float f = focoTopo[i], esc = 1.0f + 0.04f * f;
      GfxRect base = { r.x + PF_PAD_X + (pw + gap) * (float)i, y, pw, ph };
      GfxRect pr = { base.x - pw * (esc - 1.0f) * 0.5f, base.y - ph * (esc - 1.0f) * 0.5f, pw * esc, ph * esc };
      char s[96], tt[48];
      if (!o) continue;
      pfAlvoCorpo(base.x, base.y, base.w, base.h + 50.0f, ponteiroTopo, NULL, i, 0);
      if (f > 0.01f) gfx_anel_fora(pr, 12.0f / pr.h, 4.0f, 3.0f, ar, ag, ab, f * a);
      svd_poster(pr, o->poster, 12.0f / pr.h, a);
      escrever(TXT_ROW_TITULO, tituloObra(o), PF_FORTE, base.x, base.y + ph + 8.0f, pw, PF_ESC_LIN * 0.88f, a);
      tempo(tt, sizeof tt, o->minutos);
      if (o->episodios > 0) snprintf(s, sizeof s, i18n("%s · %d ep."), tt, o->episodios);
      else snprintf(s, sizeof s, i18n("%s · filme"), tt);
      escrever(TXT_CAPTION2, s, PF_APOIO, base.x, base.y + ph + 32.0f, pw, PF_ESC_APOIO * 0.9f, a);
    } }
}

// --- mix -----------------------------------------------------------------------------
static void desenharMix(float a) {
  GfxRect r = rMix();
  float ar, ag, ab, x = r.x + PF_PAD_X, w = r.w - PF_PAD_X * 2.0f, y;
  int tot = dados.minutosSeries + dados.minutosFilmes, gTot = 0, gMax = 1;
  acento(&ar, &ag, &ab);
  cartao(r, a);
  y = tituloCartao(r, "Seu mix", a) + 16.0f;
  if (tot > 0) {
    float fs = (float)dados.minutosSeries / (float)tot, ws = (w - 4.0f) * fs;
    char s[120];
    if (ws > 0.5f) gfx_cor((GfxRect){ x, y, ws, 16.0f }, 0.5f, ar, ag, ab, a);
    if (w - 4.0f - ws > 0.5f) gfx_cor((GfxRect){ x + ws + 4.0f, y, w - 4.0f - ws, 16.0f }, 0.5f, PN_FILME_R, PN_FILME_G, PN_FILME_B, a);
    snprintf(s, sizeof s, i18n("%d%% séries · %d%% filmes"), (int)(fs * 100.0f + 0.5f), 100 - (int)(fs * 100.0f + 0.5f));
    escrever(TXT_CAPTION2, s, PF_MEDIO, x, y + 26.0f, w, PF_ESC_APOIO, a);
  } else escrever(TXT_CAPTION2, "Sem minutos ainda.", PF_APOIO, x, y, w, PF_ESC_APOIO, a);
  y += 64.0f;
  for (int i = 0; i < dados.nGeneros; i++) { gTot += dados.generos[i].quantidade; if (dados.generos[i].quantidade > gMax) gMax = dados.generos[i].quantidade; }
  if (!dados.nGeneros) { escrever(TXT_CAPTION2, "Gêneros aparecem quando os títulos carregarem.", PF_APOIO, x, y, w, PF_ESC_APOIO, a); return; }
  for (int i = 0; i < dados.nGeneros && y + 30.0f < r.y + r.h - PF_PAD_Y; i++) {
    char pc[16];
    float bw = w * 0.42f, bx = x + w - bw - 48.0f;
    snprintf(pc, sizeof pc, "%d%%", gTot ? (dados.generos[i].quantidade * 100 + gTot / 2) / gTot : 0);
    escrever(TXT_CAPTION2, dados.generos[i].nome, PF_MEDIO, x, y, bx - x - 8.0f, PF_ESC_APOIO, a);
    gfx_cor((GfxRect){ bx, y + 7.0f, bw, 8.0f }, 0.5f, 1, 1, 1, .08f * a);
    gfx_cor((GfxRect){ bx, y + 7.0f, bw * (float)dados.generos[i].quantidade / (float)gMax, 8.0f }, 0.5f, ar, ag, ab, a);
    escrever(TXT_CAPTION2, pc, PF_APOIO, x + w - 40.0f, y, 40.0f, PF_ESC_APOIO, a);
    y += 30.0f;
  }
}

// --- fatos -------------------------------------------------------------------------
static void fato(int i, const char *rot, const char *val, const char *sub, float a) {
  GfxRect r = rFato(i);
  float w = r.w - 40.0f, cy = r.y + r.h * 0.5f;
  cartao(r, a);
  escrever(TXT_CAPTION2, rot, PF_MEDIO, r.x + 20.0f, cy - 40.0f, w, PF_ESC_APOIO, a);
  escrever(TXT_ROW_TITULO, val, PF_FORTE, r.x + 20.0f, cy - 16.0f, w, PF_ESC_CARD, a);
  escrever(TXT_CAPTION2, sub, PF_APOIO, r.x + 20.0f, cy + 16.0f, w, PF_ESC_APOIO * 0.95f, a);
}
static void desenharFatos(float a) {
  char v[160], s[160], t[48], d[80], rot[80];
  // MAIOR MARATONA
  if (dados.maratona.obra >= 0) {
    struct tm tm; time_t tt = (time_t)(dados.maratona.fimMs / 1000);
    localtime_r(&tt, &tm);
    // O TITULO e o valor; data e episodios no apoio ("Seg, 2 Novembro · 2
    // episodios"): "Ghosts · 2 episodios" no valor nao cabia no cartao (BlueStacks).
    pnData(d, sizeof d, &tm);
    snprintf(s, sizeof s, i18n("%s · %d episódios"), d, dados.maratona.episodios);
    fato(0, "Maior maratona", tituloObra(obraDe(dados.maratona.obra)), s, a);
  } else fato(0, "Maior maratona", "\xe2\x80\x94", "Dois episódios seguidos já contam.", a);
  // DIA MAIS ATIVO
  if (dados.diaMaisAtivo >= 0 && dados.diaMaisAtivo < 7) {
    tempo(t, sizeof t, dados.minutosSemana[dados.diaMaisAtivo]);
    snprintf(s, sizeof s, i18n("%s no total"), t);
    fato(1, "Dia mais ativo", PN_DIAS[dados.diaMaisAtivo], s, a);
  } else fato(1, "Dia mais ativo", "\xe2\x80\x94", "Sem atividade ainda.", a);
  // CONCLUSAO
  if (dados.iniciadas > 0) {
    snprintf(v, sizeof v, "%d%%", (dados.concluidas * 100 + dados.iniciadas / 2) / dados.iniciadas);
    snprintf(s, sizeof s, i18n("%d de %d terminados"), dados.concluidas, dados.iniciadas);
    fato(2, "Conclusão", v, s, a);
  } else fato(2, "Conclusão", "\xe2\x80\x94", "Nada começado nos últimos 30 dias.", a);
  // NESTE DIA
  if (dados.hojeAnos.obra >= 0) {
    struct tm tm; time_t tt = (time_t)(dados.hojeAnos.ms / 1000);
    localtime_r(&tt, &tm);
    snprintf(rot, sizeof rot, i18n("Neste dia · %d"), tm.tm_year + 1900);
    snprintf(s, sizeof s, "%d %s %d \xc2\xb7 %02d:%02d", tm.tm_mday, i18n(PN_MESES[tm.tm_mon]), tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
    fato(3, rot, tituloObra(obraDe(dados.hojeAnos.obra)), s, a);
  } else fato(3, "Neste dia", "\xe2\x80\x94", "Ainda sem histórico desta data.", a);
}

// --- a folha do dia (OK num dia) ---------------------------------------------------

static void desenharFolha(float a) {
  int d = limitar(dia, 0, PERFIL_DIAS - 1), n = limitar(dados.nDiaItens[d], 0, PERFIL_ITENS_DIA);
  float h = 120.0f + (float)n * 128.0f, w = 820.0f;
  GfxRect p = { (NV_TELA_W - w) * 0.5f, (NV_TELA_H - h) * 0.5f, w, h };
  char data[80], tt[48], l[200];
  struct tm tm = pnDia(d);
  if (!folha || n <= 0) return;
  gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0, 0, 0, 0, .55f * a);
  gfx_cor(p, 28.0f / p.h, .086f, .118f, .125f, a);
  pnData(data, sizeof data, &tm);
  tempo(tt, sizeof tt, dados.minutosDia[d]);
  snprintf(l, sizeof l, "%s \xc2\xb7 %s", data, tt);
  escrever(TXT_TITULO3, l, 245, p.x + 32.0f, p.y + 28.0f, w - 64.0f, PF_ESC_COMUM, a);
  for (int k = 0; k < n; k++) {
    const PerfilDiaItem *it = &dados.diaItens[d][k];
    const PerfilObra *o = obraDe(it->obra);
    GfxRect lr = { p.x + 20.0f, p.y + 92.0f + (float)k * 128.0f, w - 40.0f, 116.0f };
    char t2[200], sub[160], hora[16];
    struct tm ti; time_t t0 = (time_t)(it->inicioMs / 1000);
    localtime_r(&t0, &ti);
    snprintf(hora, sizeof hora, "%02d:%02d", ti.tm_hour, ti.tm_min);
    linhaFoco(lr, 18.0f / lr.h, k == folhaSel ? 1.0f : 0.0f, a);
    svd_poster((GfxRect){ lr.x + 12.0f, lr.y + 10.0f, 64.0f, 96.0f }, o ? o->poster : "", 8.0f / 96.0f, a);
    pnItem(t2, sizeof t2, it);
    tempo(tt, sizeof tt, it->minutos);
    snprintf(sub, sizeof sub, i18n("%s · começou às %s"), tt, hora);
    escrever(TXT_ROW_TITULO, t2, PF_FORTE, lr.x + 96.0f, lr.y + 32.0f, lr.w - 120.0f, PF_ESC_CARD, a);
    escrever(TXT_CAPTION2, sub, PF_APOIO, lr.x + 96.0f, lr.y + 66.0f, lr.w - 120.0f, PF_ESC_APOIO, a);
  }
}
static void pedirObra(const PerfilObra *o) {
  if (!o || !o->id[0]) return;
  memset(&dPedido, 0, sizeof dPedido);
  snprintf(dPedido.id, sizeof dPedido.id, "%s", o->id);
  snprintf(dPedido.titulo, sizeof dPedido.titulo, "%s", tituloObra(o));
  snprintf(dPedido.poster, sizeof dPedido.poster, "%s", o->poster);
  dTemPedido = 1;
}
// Teclas com a folha aberta. 1 = consumiu.
static int eventoFolha(SDL_Keycode k, int ok) {
  int d = limitar(dia, 0, PERFIL_DIAS - 1), n = limitar(dados.nDiaItens[d], 0, PERFIL_ITENS_DIA);
  if (!folha) return 0;
  if (k == SDLK_ESCAPE || k == SDLK_AC_BACK || k == SDLK_BACKSPACE || k == SDLK_DELETE) { folha = 0; return 1; }
  if (k == SDLK_UP)   { if (folhaSel > 0) folhaSel--; return 1; }
  if (k == SDLK_DOWN) { if (folhaSel + 1 < n) folhaSel++; return 1; }
  if (ok && folhaSel < n) { pedirObra(obraDe(dados.diaItens[d][folhaSel].obra)); folha = 0; return 1; }
  return 1;
}

// --- navegacao do resumo -----------------------------------------------------------
// Paradas: 0 grafico (esquerda/direita = dias), 1 cartazes, 2 pilula Amigos.
static void eventoPainel(SDL_Keycode k, int ok) {
  int temPilula = rostosCabem() > 0;
  if (secao == 0) {
    if (k == SDLK_LEFT)  { if (dia > 0) dia--; else sair = 1; return; }
    if (k == SDLK_RIGHT) { if (dia + 1 < PERFIL_DIAS) dia++; return; }
    if (k == SDLK_DOWN)  { if (nTopo()) { secao = 1; item = limitar(item, 0, nTopo() - 1); } return; }
    if (k == SDLK_UP)    { if (temPilula) secao = 2; else if (ajustes_menu_topo()) sair = 1; else pediuRelogio = 1; return; }
    if (ok && dados.nDiaItens[limitar(dia, 0, PERFIL_DIAS - 1)] > 0) { folha = 1; folhaSel = 0; }
    return;
  }
  if (secao == 1) {
    if (k == SDLK_LEFT)  { if (item > 0) item--; else sair = 1; return; }
    if (k == SDLK_RIGHT) { if (item + 1 < nTopo()) item++; return; }
    if (k == SDLK_UP)    { secao = 0; return; }
    if (ok && item < nTopo()) pedirObra(obraDe(dados.topo[item]));
    return;
  }
  if (secao == 2) {
    if (k == SDLK_DOWN) { secao = 0; return; }
    if (k == SDLK_LEFT) { sair = 1; return; }
    if (k == SDLK_UP)   { if (ajustes_menu_topo()) sair = 1; else pediuRelogio = 1; return; }
    if (ok && temPilula) { duelo(limitar(amigo, 0, socialvis_n_amigos() - 1), 1); return; }
  }
}

static void desenharLoadingPainel(Uint32 agora, float a) {
  float pulso = ajustes_animacoes_reduzidas() ? .55f : .48f + .12f * sinf((float)agora * .004f);
  float p = pulso * a;
  #define PN_ESQ(r_, raio_) gfx_cor((r_), (raio_), NV_COR_ESQUELETO_R, NV_COR_ESQUELETO_G, NV_COR_ESQUELETO_B, p)
  PN_ESQ(((GfxRect){ PF_X, PN_TOPO, PN_AV, PN_AV }), .5f);
  PN_ESQ(((GfxRect){ PF_X + PN_AV + 24, PN_TOPO + 14, 360, 40 }), NV_RAIO_BADGE);
  PN_ESQ(((GfxRect){ PF_X + PN_AV + 24, PN_TOPO + 62, 300, 18 }), NV_RAIO_BADGE);
  for (int i = 0; i < 5; i++) PN_ESQ(rKpi(i), PF_RAIO / PN_KPI_H);
  { GfxRect c[4] = { rGrafico(), rRelogio(), rTopo(), rMix() };
    for (int i = 0; i < 4; i++) PN_ESQ(c[i], PF_RAIO / c[i].h); }
  for (int i = 0; i < 4; i++) { GfxRect f = rFato(i); PN_ESQ(f, PF_RAIO / f.h); }
  #undef PN_ESQ
}

// O corpo (tudo menos o cabecalho): no retrato ele rola dentro do recorte,
// e o cabecalho fica fixo por cima (perfil_desenhar).
static void desenharPainel(Uint32 agora, float a) {
  (void)agora;
  desenharNumerosPainel(a);
  if (surge(5) > 0.002f) desenharGrafico(a * surge(5));
  if (surge(6) > 0.002f) desenharRelogio(a * surge(6));
  if (surge(7) > 0.002f) desenharTopo(a * surge(7));
  if (surge(8) > 0.002f) desenharMix(a * surge(8));
  if (surge(9) > 0.002f) desenharFatos(a * surge(9));
}
