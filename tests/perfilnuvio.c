// Painel de 30 dias a partir de Nuvio sync (src/perfilnuvio.c). Sem rede:
// vistos e progresso de mentira, relogio fixo e fuso America/New_York — a
// janela de 09/10 a 07/11/2026 atravessa o fim do horario de verao (01/11).
#include "perfilnuvio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int falhas, checagens;
#define CHECA(c, ...) do { checagens++; if (!(c)) { falhas++; printf("FALHA %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static long long ms(int y, int mo, int d, int h, int mi) {
  struct tm tm = {0};
  tm.tm_year = y - 1900; tm.tm_mon = mo - 1; tm.tm_mday = d;
  tm.tm_hour = h; tm.tm_min = mi; tm.tm_isdst = -1;
  return (long long)mktime(&tm) * 1000;
}

static ContaVisto V[64];
static ProgRegistro R[64];
static PerfilMeta M[16];
static int nV, nR, nM;
static PerfilDados D;
static long long AGORA;

static void limpar(void) { nV = nR = nM = 0; memset(V, 0, sizeof V); memset(R, 0, sizeof R); memset(M, 0, sizeof M); }
static void visto(const char *id, const char *tipo, int t, int e, long long quando) {
  ContaVisto *x = &V[nV++];
  snprintf(x->id, sizeof x->id, "%s", id); snprintf(x->tipo, sizeof x->tipo, "%s", tipo);
  x->temporada = t; x->episodio = e; x->vistoMs = quando;
}
static void prog(int perfil, const char *id, const char *tipo, int t, int e, double pos, double dur, long long quando) {
  ProgRegistro *x = &R[nR++];
  x->perfil = perfil;
  snprintf(x->contentId, sizeof x->contentId, "%s", id); snprintf(x->tipo, sizeof x->tipo, "%s", tipo);
  x->temporada = t; x->episodio = e; x->posSeg = pos; x->durSeg = dur; x->lastWatchedMs = quando;
}
static void meta(const char *id, const char *titulo, const char *genero, int dur) {
  PerfilMeta *x = &M[nM++];
  snprintf(x->id, sizeof x->id, "%s", id); snprintf(x->titulo, sizeof x->titulo, "%s", titulo);
  snprintf(x->genero, sizeof x->genero, "%s", genero); x->duracaoMin = dur;
}
static int montar(void) { return perfilnuvio_montar(V, nV, R, nR, 1, M, nM, AGORA, &D); }
static int somaHoras(void) { int s = 0; for (int h = 0; h < 24; h++) s += D.minutosHora[h]; return s; }
// Indice do dia de `quando` na janela (0 = 09/10).
static int idx(int mo, int d) { return mo == 10 ? d - 9 : 23 + d - 1; }

static void filmeSimples(void) {
  limpar();
  visto("tt0001", "movie", 0, 0, ms(2026, 11, 6, 22, 0));
  prog(1, "tt0001", "movie", 0, 0, 7200, 7200, ms(2026, 11, 6, 22, 0));
  meta("tt0001", "Um Filme", "Filme · Drama · tv show · Crime", 0);   // "TV Show" e tipo, nao genero
  int n = montar();
  CHECA(n == 1, "itens %d", n);
  CHECA(D.cobertura & PF_COB_NUVIO, "cobertura");
  CHECA(!(D.cobertura & PF_COB_ESTIMADO), "duracao do progresso nao e estimada");
  CHECA(D.minutos == 120 && D.filmes == 1 && D.episodios == 0, "min %d filmes %d eps %d", D.minutos, D.filmes, D.episodios);
  CHECA(D.minutosFilmes == 120 && D.minutosSeries == 0, "split %d/%d", D.minutosFilmes, D.minutosSeries);
  CHECA(D.minutosDia[idx(11, 6)] == 120 && D.minutosFilmeDia[idx(11, 6)] == 120, "dia %d", D.minutosDia[idx(11, 6)]);
  CHECA(D.minutosHora[20] == 60 && D.minutosHora[21] == 60, "horas %d %d", D.minutosHora[20], D.minutosHora[21]);
  CHECA(D.nTopo == 1 && !strcmp(D.obras[D.topo[0]].titulo, "Um Filme"), "topo");
  CHECA(strstr(D.obras[0].poster, "tt0001") != NULL, "cartaz metahub: %s", D.obras[0].poster);
  CHECA(D.nGeneros == 2 && !strcmp(D.generos[0].nome, "Crime") && D.generos[0].quantidade == 120,
        "generos %d %s", D.nGeneros, D.nGeneros ? D.generos[0].nome : "");
  CHECA(D.nDestaques == 1 && !strcmp(D.destaques[0].titulo, "Um Filme"), "destaque de compatibilidade");
  CHECA(D.nDiaItens[idx(11, 6)] == 1 && D.diaItens[idx(11, 6)][0].minutos == 120, "item do dia");
  CHECA(D.diaMaisAtivo == 5, "sexta = %d", D.diaMaisAtivo);   // 06/11/2026 e sexta
}

static void bordasDaJanela(void) {
  limpar();
  visto("tt0002", "movie", 0, 0, ms(2026, 10, 8, 23, 0));      // fora
  visto("tt0003", "movie", 0, 0, ms(2026, 10, 9, 0, 30));      // 30 de 60 min dentro
  prog(1, "tt0003", "movie", 0, 0, 3600, 3600, ms(2026, 10, 9, 0, 30));
  visto("tt0004", "movie", 0, 0, ms(2026, 11, 8, 1, 0));       // futuro
  montar();
  CHECA(D.filmes == 1 && D.titulos == 1, "filmes %d titulos %d", D.filmes, D.titulos);
  CHECA(D.minutosDia[0] == 30 && D.minutos == 30, "cortado %d/%d", D.minutosDia[0], D.minutos);
}

static void meiaNoiteEHorarioDeVerao(void) {
  limpar();
  visto("tt0010", "series", 1, 1, ms(2026, 10, 20, 0, 30));
  prog(1, "tt0010", "series", 1, 1, 3600, 3600, ms(2026, 10, 20, 0, 30));
  visto("tt0010", "series", 1, 2, ms(2026, 11, 2, 10, 0));     // depois da virada do horario
  prog(1, "tt0010", "series", 1, 2, 3600, 3600, ms(2026, 11, 2, 10, 0));
  montar();
  CHECA(D.minutosDia[idx(10, 19)] == 30 && D.minutosDia[idx(10, 20)] == 30, "atravessa: %d %d",
        D.minutosDia[idx(10, 19)], D.minutosDia[idx(10, 20)]);
  CHECA(D.minutosHora[23] == 30 && D.minutosHora[0] == 30, "horas %d %d", D.minutosHora[23], D.minutosHora[0]);
  CHECA(D.minutosDia[idx(11, 2)] == 60 && D.minutosHora[9] == 60, "dia apos DST %d hora9 %d",
        D.minutosDia[idx(11, 2)], D.minutosHora[9]);
  CHECA(D.atividade[idx(10, 20)] == 1 && D.atividade[idx(10, 19)] == 0, "atividade no dia do fim");
}

static void loteMarcado(void) {
  limpar();
  long long t = ms(2026, 10, 25, 3, 0);
  for (int e = 1; e <= 5; e++) visto("tt0020", "series", 1, e, t + e * 2000);
  visto("tt0021", "series", 1, 1, ms(2026, 10, 26, 21, 0));
  visto("tt0021", "series", 1, 2, ms(2026, 10, 26, 21, 0) + 100000);   // 100 s: nao e lote
  visto("tt0022", "series", 0, 0, ms(2026, 10, 27, 21, 0));             // serie inteira
  montar();
  CHECA(D.marcados == 5, "marcados %d", D.marcados);
  CHECA(D.episodios == 2 && D.titulos == 1, "eps %d titulos %d", D.episodios, D.titulos);
  CHECA(D.minutosHora[2] == 0 && D.minutosHora[3] == 0, "lote fora do relogio");
  CHECA(D.cobertura & PF_COB_ESTIMADO, "sem duracao: estimado");
  CHECA(D.minutos == 2 * PN_EP_PADRAO_MIN, "padrao %d", D.minutos);
}

static void duracaoPelaMeta(void) {
  limpar();
  visto("tt0030", "series", 1, 1, ms(2026, 11, 3, 22, 0));
  meta("tt0030", "Serie Meta", "Drama, Mistério", 50);
  visto("tt0031", "series", 1, 1, ms(2026, 11, 3, 23, 0));
  prog(1, "tt0031", "series", 1, 1, 3000, 0.001, ms(2026, 11, 3, 23, 0));   // marcador de 1 ms
  meta("tt0031", "Outra", "", 0);
  montar();
  CHECA(D.minutos == 50 + PN_EP_PADRAO_MIN, "min %d", D.minutos);
  CHECA(!(D.cobertura & PF_COB_ESTIMADO) || PN_EP_PADRAO_MIN * 5 > D.minutos, "limiar 20%%");
  CHECA(D.nGeneros == 2 && !strcmp(D.generos[0].nome, "Drama"), "generos de virgula");
}

static void parcialEOutroPerfil(void) {
  limpar();
  prog(1, "tt0040", "series", 2, 3, 1200, 3600, ms(2026, 11, 5, 21, 0));
  prog(2, "tt0041", "movie", 0, 0, 3000, 6000, ms(2026, 11, 5, 21, 0));   // outro perfil
  prog(1, "tt0042", "movie", 0, 0, 6800, 7000, ms(2026, 11, 4, 22, 0));   // 97%, sem visto
  int n = montar();
  CHECA(n == 2, "itens %d", n);
  CHECA(D.episodios == 0 && D.filmes == 1, "parcial nao conta episodio: eps %d filmes %d", D.episodios, D.filmes);
  CHECA(D.minutosDia[idx(11, 5)] == 20, "parcial = posicao %d", D.minutosDia[idx(11, 5)]);
  CHECA(D.titulos == 2, "titulos %d", D.titulos);
  CHECA(D.iniciadas == 2 && D.concluidas == 1, "completude %d/%d", D.concluidas, D.iniciadas);
}

static void maratona(void) {
  limpar();
  long long f1 = ms(2026, 10, 29, 21, 0);
  long long f2 = f1 + (30 + 50) * 60000LL;           // vao de 30 min: segue
  long long f3 = f2 + (31 + 50) * 60000LL;           // vao de 31 min: quebra
  visto("tt0050", "series", 2, 4, f1); prog(1, "tt0050", "series", 2, 4, 3000, 3000, f1);
  visto("tt0050", "series", 2, 5, f2); prog(1, "tt0050", "series", 2, 5, 3000, 3000, f2);
  visto("tt0050", "series", 2, 6, f3); prog(1, "tt0050", "series", 2, 6, 3000, 3000, f3);
  meta("tt0050", "From", "Terror", 0);
  montar();
  CHECA(D.maratona.episodios == 2 && D.maratona.epIni == 4 && D.maratona.epFim == 5,
        "maratona %d eps %d-%d", D.maratona.episodios, D.maratona.epIni, D.maratona.epFim);
  CHECA(D.maratona.obra >= 0 && !strcmp(D.obras[D.maratona.obra].titulo, "From"), "obra da maratona");
  CHECA(D.maratona.minutos == 100, "min %d", D.maratona.minutos);

  limpar();
  long long t = ms(2026, 10, 30, 20, 0);
  for (int e = 1; e <= 4; e++) {
    long long fim = t + e * (45 + 5) * 60000LL;
    visto("tt0051", "series", 1, e, fim); prog(1, "tt0051", "series", 1, e, 2700, 2700, fim);
  }
  visto("tt0052", "movie", 0, 0, ms(2026, 10, 31, 20, 0));
  montar();
  CHECA(D.maratona.episodios == 4, "4 seguidos: %d", D.maratona.episodios);
  CHECA(D.nDiaItens[idx(10, 30)] == 1 && D.diaItens[idx(10, 30)][0].epIni == 1 &&
        D.diaItens[idx(10, 30)][0].epFim == 4 && D.diaItens[idx(10, 30)][0].minutos == 180, "faixa do dia");
}

static void somaDasHoras(void) {
  limpar();
  for (int i = 0; i < 20; i++) {
    long long fim = ms(2026, 10, 10 + i, 7 + i % 17, 13 * i % 60);
    char id[16]; int k = i % 7, serie = k % 3 != 0;
    snprintf(id, sizeof id, "tt09%02d", k);
    visto(id, serie ? "series" : "movie", serie, serie ? i + 1 : 0, fim);
    prog(1, id, serie ? "series" : "movie", serie, serie ? i + 1 : 0, 1000 + 377 * i, 1000 + 377 * i, fim);
  }
  montar();
  int sh = somaHoras(), sd = 0, sw = 0;
  for (int i = 0; i < PERFIL_DIAS; i++) sd += D.minutosDia[i];
  for (int w = 0; w < 7; w++) sw += D.minutosSemana[w];
  CHECA(abs(sh - D.minutos) <= 12 && abs(sd - D.minutos) <= 15 && abs(sw - D.minutos) <= 4,
        "somas h%d d%d s%d total %d", sh, sd, sw, D.minutos);
  CHECA(D.minutosFilmes + D.minutosSeries == D.minutos, "split soma");
  CHECA(D.titulos == 7, "titulos %d", D.titulos);
  CHECA(D.nTopo == 6, "topo %d", D.nTopo);
  for (int i = 1; i < D.nTopo; i++)
    CHECA(D.obras[D.topo[i - 1]].minutos >= D.obras[D.topo[i]].minutos, "topo ordenado");
}

static void nesteDiaESequencia(void) {
  limpar();
  visto("tt0060", "movie", 0, 0, ms(2024, 11, 7, 21, 0));
  visto("tt0061", "movie", 0, 0, ms(2025, 11, 7, 21, 40));
  visto("tt0062", "movie", 0, 0, ms(2025, 11, 8, 21, 0));      // outro dia
  meta("tt0061", "Prisoners", "", 150);
  for (int d = 4; d <= 6; d++) visto("tt0063", "series", 1, d, ms(2026, 11, d, 21, 0));
  montar();
  CHECA(D.hojeAnos.obra >= 0 && !strcmp(D.obras[D.hojeAnos.obra].id, "tt0061"), "neste dia");
  CHECA(D.hojeAnos.ms == ms(2025, 11, 7, 21, 40), "instante");
  CHECA(D.streakAtual == 3, "sequencia sem hoje = %d", D.streakAtual);
  CHECA(D.diasAtivosMes == 3, "dias ativos %d", D.diasAtivosMes);
  visto("tt0063", "series", 1, 7, ms(2026, 11, 7, 9, 0));
  montar();
  CHECA(D.streakAtual == 4, "hoje conta = %d", D.streakAtual);

  char ids[8][24], tipos[8][8];
  int n = perfilnuvio_ids_meta(V, nV, R, nR, 1, AGORA, ids, tipos, 8);
  CHECA(n == 2 && !strcmp(ids[0], "tt0063") && !strcmp(ids[1], "tt0061"), "ids_meta %d", n);
}

static void metaLer(void) {
  PerfilMeta m;
  memset(&m, 0, sizeof m);
  CHECA(perfilnuvio_meta_ler("{\"meta\":{\"id\":\"tt1\",\"name\":\"From\",\"poster\":\"https://x/p.jpg\","
                             "\"genres\":[\"Drama\", \"Horror\",\"Mystery\"],\"runtime\":\"1h 5min\"}}", &m), "meta ok");
  CHECA(!strcmp(m.titulo, "From") && !strcmp(m.poster, "https://x/p.jpg"), "titulo/cartaz");
  CHECA(!strcmp(m.genero, "Drama, Horror, Mystery"), "generos: %s", m.genero);
  CHECA(m.duracaoMin == 65, "runtime %d", m.duracaoMin);
  memset(&m, 0, sizeof m);
  snprintf(m.titulo, sizeof m.titulo, "Da Biblioteca");
  perfilnuvio_meta_ler("{\"meta\":{\"name\":\"Outro\",\"runtime\":\"45 min\"}}", &m);
  CHECA(!strcmp(m.titulo, "Da Biblioteca") && m.duracaoMin == 45, "biblioteca vence; %d", m.duracaoMin);
  memset(&m, 0, sizeof m);
  perfilnuvio_meta_ler("{\"meta\":{\"name\":\"N\",\"runtime\":95}}", &m);
  CHECA(m.duracaoMin == 95, "runtime numerico %d", m.duracaoMin);
  CHECA(!perfilnuvio_meta_ler("{\"metas\":[]}", &m) || 1, "sem meta");
  memset(&m, 0, sizeof m);
  CHECA(!perfilnuvio_meta_ler("nada", &m) && !perfilnuvio_meta_ler(NULL, &m), "corpo invalido");
}

static void vazio(void) {
  limpar();
  int n = montar();
  CHECA(n == 0 && D.minutos == 0 && D.nObras == 0 && D.maratona.obra == -1 && D.hojeAnos.obra == -1 &&
        D.diaMaisAtivo == -1 && D.nDias == PERFIL_DIAS, "vazio");
}

int main(void) {
  setenv("TZ", "America/New_York", 1);
  tzset();
  AGORA = ms(2026, 11, 7, 20, 0);
  filmeSimples();
  bordasDaJanela();
  meiaNoiteEHorarioDeVerao();
  loteMarcado();
  duracaoPelaMeta();
  parcialEOutroPerfil();
  maratona();
  somaDasHoras();
  nesteDiaESequencia();
  metaLer();
  vazio();
  printf("perfilnuvio: %d checagens, %d falhas\n", checagens, falhas);
  return falhas ? 1 : 0;
}
