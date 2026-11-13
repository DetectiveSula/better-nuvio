#include "servidores.h"
/* Full settings sync, stages 2b/3 (owner, 12 Nov 2026): settings, keys and logins
 * kept in data files (debrid/TMDB/Fanart/Seekr/poster keys, P2P address, failure
 * policy with filters and orders, add-on order, Live TV accounts and lists,
 * Jellyfin/Emby/Plex servers). The owner asked for keys and logins to sync too.
 *
 * Each file is carried as base64url text split over JT_PARTES[i] string fields
 * ("<key>0", "<key>1", ...): the engine refuses control characters, newlines and
 * "://" in values and secret-looking field names, so content is encoded, not
 * encrypted. A file that would not fit is left out (logged by name only).
 *
 * Same rules as the bindings: a change made on this TV uploads; the account's
 * value applies when this TV has no unsent change; a device-wide file new to
 * this profile uploads once; per-profile files also catch offline edits through
 * a per-profile anchor of content hashes (never the content). */
#define JT_PARTE 150
#define JT_CHAVE_MAX 40
typedef struct {
  const char *chave;                    /* wire key prefix (no secret words) */
  int partes;                           /* number of string fields */
  int (*nome)(char *dst, unsigned n);   /* data file name; NULL = custom ler/gravar */
  int (*ler)(char *dst, size_t cap);    /* custom: raw bytes, -1 = do not sync */
  int (*gravar)(const char *texto);     /* custom writer */
  void (*recarregar)(void);
  int dispositivo;                      /* device-wide store (see header) */
} JtArq;

static int jtNomeFixo(char *d, unsigned n, const char *f) { return snprintf(d, n, "%s", f) < (int)n; }
static int jtNomeDebrid(char *d, unsigned n) { return ajustes_provedores_arquivo(d, n, "debrid"); }
static int jtNomeFalha(char *d, unsigned n) { return ajustes_provedores_arquivo(d, n, "fontefalha"); }
static int jtNomeTmdb(char *d, unsigned n) { return debNomePerfil(d, n, "tmdb-key"); }
static int jtNomeFanart(char *d, unsigned n) { return jtNomeFixo(d, n, "fanart.txt"); }
static int jtNomeSeekr(char *d, unsigned n) { return jtNomeFixo(d, n, "seekr.txt"); }
static int jtNomePosters(char *d, unsigned n) { return jtNomeFixo(d, n, "posteres.txt"); }
static int jtNomeP2p(char *d, unsigned n) { return jtNomeFixo(d, n, "p2p.txt"); }
static int jtNomePerfil(char *d, unsigned n, const char *fmt) { int p = perfis_ativo(); return p > 0 && snprintf(d, n, fmt, p) < (int)n; }
static int jtNomeUrls(char *d, unsigned n) { return jtNomePerfil(d, n, "ltv-urls-p%d.txt"); }
static int jtNomeJf(char *d, unsigned n) { return jtNomePerfil(d, n, "jellyfin-p%d.txt"); }
static int jtNomeEmby(char *d, unsigned n) { return jtNomePerfil(d, n, "emby-p%d.txt"); }
static int jtNomePlex(char *d, unsigned n) { return jtNomePerfil(d, n, "plex-p%d.txt"); }
static void jtRecUrls(void) { ltv_urls_carregar(perfis_ativo()); }
static void jtRecFalha(void) { fontefalha_config_ler(); ajFalhaEspelhar(); }
static void jtRecIptv(void) { iptv_contas_recarregar(); ajIptvEspelhar(); guia_recarregar(1); }

/* Add-on order: "a › b › c" (fonteregra_ordem_texto). */
#define JT_SEP " \xE2\x80\xBA "
static int jtLerOrdem(char *d, size_t cap) { fonteregra_ordem_texto(d, cap); return strlen(d) + 1 >= cap ? -1 : (int)strlen(d); }
static int jtGravarOrdem(const char *t) {
  enum { MAX = 64 };
  char buf[4096], *nomes[MAX]; int n = 0;
  snprintf(buf, sizeof buf, "%s", t ? t : "");
  for (char *p = buf; *p && n < MAX;) {
    char *s = strstr(p, JT_SEP);
    if (s) *s = 0;
    if (*p) nomes[n++] = p;
    if (!s) break;
    p = s + strlen(JT_SEP);
  }
  fonteregra_ordem_definir((const char *const *)nomes, n);
  return 1;
}

/* Live TV accounts: the profile's index plus each account's credential file,
 * bundled as "@@bn-file <name> <len>\n<bytes>". Caches are not carried. */
static int jtIptvDono(const char *n, int p) {
  char a[32], b[32], c[32], x[40];
  snprintf(x, sizeof x, "livetv-contas-p%d.txt", p);
  if (!strcmp(n, x)) return 1;
  snprintf(a, sizeof a, "xtream-p%d", p); snprintf(b, sizeof b, "m3u-p%d", p); snprintf(c, sizeof c, "stalker-p%d", p);
  for (int k = 0; k < 3; k++) {
    const char *pre = k == 0 ? a : k == 1 ? b : c; size_t l = strlen(pre);
    if (strncmp(n, pre, l)) continue;
    if (!strcmp(n + l, ".txt")) return 1;
    if (n[l] == '-' && n[l + 1] == 's' && strspn(n + l + 2, "0123456789") > 0 && !strcmp(n + l + 2 + strspn(n + l + 2, "0123456789"), ".txt")) return 1;
  }
  return 0;
}
static int jtCmp(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static int jtIptvNomes(char nomes[][64], int max) {
  int p = perfis_ativo(), n = 0;
  DIR *dir = p > 0 ? opendir(dados_dir()) : NULL;
  if (!dir) return 0;
  struct dirent *e;
  while ((e = readdir(dir)) && n < max)
    if (strlen(e->d_name) < 64 && jtIptvDono(e->d_name, p)) snprintf(nomes[n++], 64, "%s", e->d_name);
  closedir(dir);
  char *ptr[64]; for (int i = 0; i < n; i++) ptr[i] = nomes[i];
  qsort(ptr, (size_t)n, sizeof *ptr, jtCmp);
  char tmp[64][64]; for (int i = 0; i < n; i++) snprintf(tmp[i], 64, "%s", ptr[i]);
  for (int i = 0; i < n; i++) snprintf(nomes[i], 64, "%s", tmp[i]);
  return n;
}
static int jtLerIptv(char *d, size_t cap) {
  char nomes[64][64]; int n = jtIptvNomes(nomes, 64); size_t w = 0;
  d[0] = 0;
  for (int i = 0; i < n; i++) {
    char *t = dados_ler(nomes[i]); size_t l = t ? strlen(t) : 0;
    int k = snprintf(d + w, cap - w, "@@bn-file %s %zu\n", nomes[i], l);
    if (k < 0 || w + (size_t)k + l + 1 >= cap) { if (t) { memset(t, 0, l); free(t); } return -1; }
    w += (size_t)k; memcpy(d + w, t ? t : "", l); w += l; d[w] = 0;
    if (t) { memset(t, 0, l); free(t); }
  }
  return (int)w;
}
static int jtGravarIptv(const char *t) {
  int p = perfis_ativo(); if (p <= 0) return 0;
  char recebidos[64][64]; int nr = 0;
  for (const char *c = t ? t : ""; *c;) {
    char nome[64]; size_t l; int used = 0;
    if (sscanf(c, "@@bn-file %63s %zu\n%n", nome, &l, &used) != 2 || !used || strlen(c + used) < l || !jtIptvDono(nome, p)) return 0;
    char *corpo = strndup(c + used, l); if (!corpo) return 0;
    int ok = l ? debGravarArquivo(nome, corpo) : dados_apagar(nome);
    memset(corpo, 0, l); free(corpo);
    if (!ok) return 0;
    if (nr < 64) snprintf(recebidos[nr++], 64, "%s", nome);
    c += used + l;
  }
  /* Accounts removed on the other TV: remove their files here too. */
  char locais[64][64]; int nl = jtIptvNomes(locais, 64);
  for (int i = 0; i < nl; i++) {
    int tem = 0; for (int k = 0; k < nr; k++) if (!strcmp(locais[i], recebidos[k])) tem = 1;
    if (!tem) dados_apagar(locais[i]);
  }
  return 1;
}

static const JtArq jtArq[] = {
  { "fileDebrid",      8, jtNomeDebrid,  NULL, NULL, debCarregar,          1 },
  { "fileTmdbKey",     2, jtNomeTmdb,    NULL, NULL, tmdbLocalCarregar,    0 },
  { "fileFanart",      2, jtNomeFanart,  NULL, NULL, fanartCarregar,       1 },
  { "fileSeekr",       2, jtNomeSeekr,   NULL, NULL, seekrCarregar,        1 },
  { "filePosters",     8, jtNomePosters, NULL, NULL, pstCarregar,          1 },
  { "fileP2p",         4, jtNomeP2p,     NULL, NULL, p2pCarregar,          1 },
  { "fileFailure",    24, jtNomeFalha,   NULL, NULL, jtRecFalha,           1 },
  { "fileAddonOrder", 16, NULL, jtLerOrdem, jtGravarOrdem, NULL,           0 },
  { "fileLiveTv",     40, NULL, jtLerIptv,  jtGravarIptv,  jtRecIptv,      0 },
  { "fileLiveTvLists",16, jtNomeUrls,    NULL, NULL, jtRecUrls,            0 },
  { "fileJellyfin",    8, jtNomeJf,      NULL, NULL, servidores_perfil_trocou, 0 },
  { "fileEmby",        8, jtNomeEmby,    NULL, NULL, servidores_perfil_trocou, 0 },
  { "filePlex",        8, jtNomePlex,    NULL, NULL, servidores_perfil_trocou, 0 },
};
#define JT_N (sizeof jtArq / sizeof *jtArq)
#define JT_PARTES_MAX 40
#define JT_DEFS_MAX (JT_N * JT_PARTES_MAX)
/* Raw capacity of one file: base64 of 3 bytes is 4 characters. */
static size_t jtCap(size_t i) { return (size_t)jtArq[i].partes * JT_PARTE / 4 * 3; }
static size_t jtDefsN(void) { size_t n = 0; for (size_t i = 0; i < JT_N; i++) n += (size_t)jtArq[i].partes; return n; }
static void jtChave(size_t i, int parte, char *d) { snprintf(d, JT_CHAVE_MAX, "%s%d", jtArq[i].chave, parte); }

/* Raw content into `d` (cap bytes); -1 = not syncable now. */
static int jtLer(size_t i, char *d, size_t cap) {
  const JtArq *a = &jtArq[i];
  if (a->ler) return a->ler(d, cap);
  char nome[160];
  if (!a->nome(nome, sizeof nome)) return -1;
  char *t = dados_ler(nome);
  size_t l = t ? strlen(t) : 0;
  if (l + 1 > cap) { if (t) { memset(t, 0, l); free(t); } return -1; }
  memcpy(d, t ? t : "", l + 1);
  if (t) { memset(t, 0, l); free(t); }
  return (int)l;
}
static int jtGravar(size_t i, const char *t) {
  const JtArq *a = &jtArq[i]; int ok;
  if (a->gravar) ok = a->gravar(t);
  else {
    char nome[160];
    if (!a->nome(nome, sizeof nome)) return 0;
    ok = *t ? debGravarArquivo(nome, t) : (dados_apagar(nome), 1);
  }
  if (ok && a->recarregar) a->recarregar();
  return ok;
}

static const char JT_B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static size_t jtB64(const unsigned char *s, size_t n, char *d) {
  size_t w = 0;
  for (size_t i = 0; i < n; i += 3) {
    unsigned v = (unsigned)s[i] << 16 | (i + 1 < n ? (unsigned)s[i + 1] << 8 : 0) | (i + 2 < n ? s[i + 2] : 0);
    d[w++] = JT_B64[v >> 18 & 63]; d[w++] = JT_B64[v >> 12 & 63];
    if (i + 1 < n) d[w++] = JT_B64[v >> 6 & 63];
    if (i + 2 < n) d[w++] = JT_B64[v & 63];
  }
  d[w] = 0; return w;
}
static int jtB64Dec(const char *s, unsigned char *d, size_t cap, size_t *out) {
  unsigned v = 0; int bits = 0; size_t w = 0;
  for (; *s; s++) {
    const char *p = strchr(JT_B64, *s);
    if (!p || !*s) return 0;
    v = v << 6 | (unsigned)(p - JT_B64); bits += 6;
    if (bits >= 8) { bits -= 8; if (w + 1 >= cap) return 0; d[w++] = (unsigned char)(v >> bits & 255); }
  }
  d[w] = 0; *out = w; return 1;
}
static int jtAceita(const BnValue *v) { return strlen(v->text) <= JT_PARTE && strspn(v->text, JT_B64) == strlen(v->text); }

/* Content -> part values (base64url, JT_PARTE characters per part). */
static int jtPartes(size_t i, const char *raw, size_t l, BnValue *vals) {
  size_t cap = (size_t)jtArq[i].partes * JT_PARTE + 4;
  char *b = malloc(cap); if (!b) return 0;
  size_t n = jtB64((const unsigned char *)raw, l, b);
  if (n > (size_t)jtArq[i].partes * JT_PARTE) { free(b); return 0; }
  for (int k = 0; k < jtArq[i].partes; k++) {
    memset(&vals[k], 0, sizeof vals[k]); vals[k].type = BN_STRING;
    size_t off = (size_t)k * JT_PARTE;
    if (off < n) { size_t m = n - off < JT_PARTE ? n - off : JT_PARTE; memcpy(vals[k].text, b + off, m); vals[k].text[m] = 0; }
  }
  free(b); return 1;
}
/* Part values -> content; 0 when malformed. */
static int jtJuntar(size_t i, const BnValue *vals, char *raw, size_t cap, size_t *l) {
  size_t tcap = (size_t)jtArq[i].partes * JT_PARTE + 1;
  char *b = malloc(tcap); if (!b) return 0;
  b[0] = 0; size_t w = 0;
  for (int k = 0; k < jtArq[i].partes; k++) { size_t m = strlen(vals[k].text); memcpy(b + w, vals[k].text, m); w += m; b[w] = 0; }
  int ok = jtB64Dec(b, (unsigned char *)raw, cap, l);
  free(b); return ok;
}
static uint64_t jtHash(const char *s, size_t n) {
  uint64_t h = 1469598103934665603ULL;
  for (size_t k = 0; k < n; k++) h = (h ^ (unsigned char)s[k]) * 1099511628211ULL;
  return h;
}

/* Fill the string definitions after the bindings. Initial value is empty: a
 * field is unobserved until this TV or the account says otherwise. */
static size_t jtDefs(BnDef *defs) {
  size_t w = 0;
  for (size_t i = 0; i < JT_N; i++)
    for (int k = 0; k < jtArq[i].partes; k++) {
      BnDef *d = &defs[w++]; memset(d, 0, sizeof *d);
      jtChave(i, k, d->key); d->type = BN_STRING; d->portable = 1; d->initial.type = BN_STRING; d->accept = jtAceita;
    }
  return w;
}
static int jtChaveSincronizavel(const char *k) {
  char c[JT_CHAVE_MAX];
  for (size_t i = 0; i < JT_N; i++) for (int p = 0; p < jtArq[i].partes; p++) { jtChave(i, p, c); if (!strcmp(c, k)) return 1; }
  return 0;
}

/* Per-profile anchor of content hashes (never content). */
static void jtAnchorNome(const char *owner, int profile, char *d, size_t n) { snprintf(d, n, "bn-files-v1-%s-p%d.txt", owner, profile); }
static int jtAnchorLer(const char *owner, int profile, uint64_t *h, unsigned char *tem) {
  char nome[160]; jtAnchorNome(owner, profile, nome, sizeof nome);
  memset(tem, 0, JT_N);
  char *t = dados_ler(nome); if (!t) return 0;
  for (char *l = t; l && *l;) {
    char *f = strchr(l, '\n'); if (f) *f++ = 0;
    char k[JT_CHAVE_MAX]; unsigned long long v;
    if (sscanf(l, "%39s %llx", k, &v) == 2)
      for (size_t i = 0; i < JT_N; i++) if (!strcmp(k, jtArq[i].chave)) { h[i] = v; tem[i] = 1; }
    l = f;
  }
  free(t); return 1;
}
static void jtAnchorGravar(const char *owner, int profile, const uint64_t *h, const unsigned char *tem) {
  char nome[160], buf[JT_N * 64 + 1]; size_t w = 0; buf[0] = 0;
  jtAnchorNome(owner, profile, nome, sizeof nome);
  for (size_t i = 0; i < JT_N; i++) if (tem[i]) w += (size_t)snprintf(buf + w, sizeof buf - w, "%s %016llx\n", jtArq[i].chave, (unsigned long long)h[i]);
  dados_gravar(nome, buf);
}

/* Record a file's current content as an edit of all its parts. */
static int jtEditar(BnAccount *acc, size_t i, const char *raw, size_t l) {
  BnValue vals[JT_PARTES_MAX]; char keys[JT_PARTES_MAX][JT_CHAVE_MAX]; const char *kp[JT_PARTES_MAX];
  if (!jtPartes(i, raw, l, vals)) return 0;
  for (int k = 0; k < jtArq[i].partes; k++) { jtChave(i, k, keys[k]); kp[k] = keys[k]; }
  return bna_edit_many(acc, kp, vals, (size_t)jtArq[i].partes);
}
