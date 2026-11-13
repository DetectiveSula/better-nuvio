#include "posplay.h"
#include "intro.h"
#include "proximo.h"
#include "epdata.h"
#include "fontecache.h"
#include "proximopreparo.h"
#include "jfid.h"
#include "idioma.h"
#include "catalogo.h"
#include "extras.h"
#include "posterprov.h"
#include "gfx.h"
#include "text.h"
#include "sinopse.h"
#include "tex_cache.h"
#include "layout.h"
#include "anim.h"
#include "ajustes.h"
#include "settings_expansion_policy.h"
#include "postplay_preview.h"
#include "vistoep.h"
#include "detail.h"
#include "descoberta.h"
#include "video.h"
#include "vistoep.h"
#include "player.h"
#include "plrui.h"
#include "plrilha.h"
#include "velocidade.h"
#define NV_ESCALA_TELA   // o arquivo inteiro mede pela tela virtual (escala.h)
#include "escala.h"
#include "ponteiro.h"
#include "rolagemtoque.h"
#include "telefoneui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Contagem configuravel, 5 s de fabrica, desligada por padrao (#256).
// O web abre o painel de filme aos 90%; aqui, desde a 2.0.3, sem marcador e um
// tempo FIXO antes do fim (intro_fim_estimado_filme, intro.h).
// #470: a contagem comeca quando o cartao sobe, ainda durante os creditos.
// Guardas do #115 ("More like this aparece cedo demais — no comeco do
// filme"). Ver posplay_regra_filme.
#define PP_FILME_METADE   0.5     // nunca antes da metade da duracao
#define PP_DUR_ESTAVEL_S  8.0     // duracao parada ha tanto tempo, no minimo
// "Fim real" do arquivo para a cena pos-creditos: o pipeline para a uma fracao
// de segundo da duracao (video_pos nao chega exatamente nela), entao um >=
// exato nunca voltaria a abrir o cartao.
#define PP_FIM_TOL_S      1.5

static int    visivel, serie, idx = -1, foco;
static int    botaoFilme;           // film page, TV: PB_ABRIR/PB_ANT/PB_PROX/PB_JANELA
static float relTroca = 1.0f;   // 0 -> 1 after ◀ / ▶ (crossfade of the text)
static int relAnterior = -1;    // backdrop under the crossfade
static GfxRect janelaReal;      // the receded video, in real pixels (posplay_fundo)
#define PP_COL_X     112.0f   // overscanHorizontal 56 dp
#define PP_COL_BASE   72.0f   // overscanVertical 36 dp
#define PP_BT_H       96.0f   // 14 dp padding + 20 sp line, both sides
#define PP_SETA       96.0f   // xxxl 48 dp
#define PP_VAO        24.0f   // md 12 dp
#define PP_LOGO_H    184.0f   // 92 dp
#define PP_CARD_W2   840.0f   // 420 dp
#define PP_THUMB_W2  224.0f   // 112 dp
#define PP_THUMB_H2  128.0f   //  64 dp
enum { PB_ABRIR, PB_ANT, PB_PROX, PB_JANELA };

static int nRel(void) {
  int n = extras_n_relacionados();
  return n > EX_REL_MAX ? EX_REL_MAX : n;
}
#ifdef NV_TOUCH_UI
static ToqueRolagem toquePP;
static float scrollPP;
static int toquePPRolar(const PonteiroRolagem *e) {
  return visivel && !serie && telefoneui_ativo() ? toquerol_evento(&toquePP, e) : 0;
}
#endif
// O titulo de `idx` (#190). O catalogo e refeito com o player aberto e a mesma
// posicao passa a ser de outro titulo: sem o id, o cartao A seguir mostrava o
// episodio de outra serie. Ver fixarTitulo.
static char   idTitulo[64];
// DURACAO ESTAVEL. O player passa o que tiver: a duracao do metadado, a
// reserva de 114 min e, assim que o pipeline responde, a dele — e no primeiro
// instante o pipeline pode informar uma duracao pequena e provisoria (o
// arquivo ainda chegando — hipotese do conserto de 10/09, nao medida aqui).
// Com uma duracao de 30 s, a metade e o fim do filme chegam aos 15 s, e a
// regra de 2 min da serie vale na hora. Decidir so depois de a duracao ficar
// PARADA por PP_DUR_ESTAVEL_S tira esse instante do jogo sem atrasar o fim de
// verdade.
static double durVista;
static double durEstavel;

// DISPENSADO GRUDA. Sem isto o Voltar fechava o painel e o quadro seguinte o
// reabria na hora, porque a condicao de aparecer (passar de 90% do filme)
// continua verdadeira ate o fim — foi o "nao da pra sair, quebra tudo" que o
// dono viu. So volta a valer quando a reproducao SAI da zona, ou quando o
// player abre outro titulo.
static int    dispensado;
static float  anim;
static Uint32 fecharEm;              // 0 = sem contagem
static int movieRecommendationState=-1;
static int    pedT, pedE, pedTitulo = -1;
static int    pedAutomatico, automaticoEntregue, recuou, aguardaBusca;
static int    proxT, proxE;          // proximo episodio, quando ha
static char   proxNome[120];
static int    proxFuturo;          // o proximo ainda nao foi ao ar: so avisa a data
static unsigned watchedAutomatically;
static int unattendedCarry,watchedFromT,watchedFromE,watchedNextT,watchedNextE;
static char watchedNextTitle[64];
static int watchedInterval=-1;
static int stillWatchingPrompt;
static char lastAutoCounted[256];
static NvPerfilSessao watchScope;
// Temporada0 e especial; quem distingue ausencia e o numero do episodio.
static int temProximo(void) { return serie && proxT >= 0 && proxE > 0; }
// O proximo que da para TOCAR: existe e ja foi ao ar.
static int temProximoJogavel(void) { return temProximo() && !proxFuturo; }

int posplay_episodio_futuro(const CatEp *ep, long long agoraMs) {
  long long quando;
  if (!ep) return 0;
  if (ep->dataIso[0]) return ep_ja_saiu(ep->dataIso, agoraMs) == 0;
  quando = prox_episodio_data_ms(ep);   // pacote legado: so a data por extenso
  return quando != PROX_SEM_DATA && quando > agoraMs;
}
static int temPedidoEpisodio(void) { return pedT >= 0 && pedE > 0; }
static char   preEpisodio[64], preAlvo[64];
static int    preTentou;
static int    preVideoTentou, preVideoRenovou;
static FontecacheEscopo preEscopo;

static void cancelarPreparo(int apagar) {
  fontecache_cancelar_proximo();
  if (apagar && preAlvo[0])
    fontecache_vod_apagar(preAlvo, "series", "", &preEscopo);
}
// CENA POS-CREDITOS (TMDB 179430/179431 em CatItem.posCreditos). `aguardaCena`:
// ha cena e o arquivo ainda nao acabou — o cartao fica no ar mas NAO age (nem
// contagem, nem troca de titulo). `cenaPendente`: a pessoa escolheu continuar;
// o cartao volta inteiro no fim real. So a acao da UI espera: visto/progresso
// seguem no limiar de creditos (player.c, concluiuAgora).
static int    cenaPendente, cenaNoFim, aguardaCena;
int posplay_visivel(void) { return visivel; }
// Preserve a pending card/Next; a dismissed card leaves no choice at EOF.
int posplay_aguarda_escolha(void) {
  return !dispensado || visivel || temPedidoEpisodio() || automaticoEntregue;
}
// Cartao do proximo episodio: SOBRE o video em tela cheia (o video nao recua).
// Os relacionados do filme, bem mais altos, continuam recuando o video.
int posplay_sobre_video(void) { return visivel && (serie || aguardaCena); }

// SAIR DA TELA PRESERVANDO A ESCOLHA, e a diferenca com posplay_fechar e o
// issue #14 inteiro.
//
// posplay_fechar zera pedT/pedE/pedTitulo, e isso e certo no caso dele: o
// player abriu outro titulo e o pedido do anterior nao vale mais. Mas os
// tratadores de tecla ARMAVAM o pedido e chamavam posplay_fechar na linha
// seguinte, que apagava o pedido recem-armado. posplay_pediu_episodio nunca
// devolvia nada, e o OK do cartao de proximo episodio NUNCA funcionou, em
// nenhuma plataforma. A correcao de id do v1.0.10 (app.c, o `strcspn` no
// `ci->imdb` composto) fica a jusante deste portao: codigo certo que nunca
// rodava, o que explica o relato de que "o fix do 1.0.10 nao resolveu".
//
// `dispensado` vem junto e nao e detalhe. Sem ele a condicao de aparecer
// (janelaSerie) continua verdadeira e o painel volta no quadro seguinte —
// no video do relato o cartao nem sequer some depois do OK, que era este
// segundo defeito somado ao primeiro.
static void esconder(void) {
  postplay_preview_close();
  automaticoEntregue = 0;
  cancelarPreparo(!temPedidoEpisodio());
  // O pedido Next consome no roteador antes de encerrar o player. Back/Down
  // descartam; uma escolha explicita de Next conserva o mesmo worker/token.
  if (!temPedidoEpisodio()) proximopreparo_cancelar();
  visivel = 0; fecharEm = 0; foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f; dispensado = 1;
}

void posplay_fechar(void) {
  movieRecommendationState=-1;
  /* Automatic Next closes then reopens the player. Only that exact transition
   * carries the episode counter; a manual stop/open begins a new sequence. */
  if(!unattendedCarry){watchedAutomatically=0;lastAutoCounted[0]=0;}
  postplay_preview_close();
  stillWatchingPrompt=0;
  // Uma lista JA completa continua disponivel ao buscarParaPlayer do proximo.
  // O fio em voo e cancelado sem esperar HTTP na UI; nunca publica depois daqui.
  cancelarPreparo(0);
  proximopreparo_cancelar();
  preTentou = 0; preEpisodio[0] = preAlvo[0] = 0;
  preVideoTentou = preVideoRenovou = 0;
  visivel = 0; fecharEm = 0; foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f;
  cenaPendente = cenaNoFim = aguardaCena = 0;
  durVista = 0.0; durEstavel = 0.0;   // titulo novo: a duracao comeca de novo
  pedT = pedE = 0; pedTitulo = -1;
  pedAutomatico = automaticoEntregue = recuou = aguardaBusca = 0;
  dispensado = 0;   // titulo novo: a dispensa do anterior nao vale mais
#ifdef NV_TOUCH_UI
  scrollPP = 0; memset(&toquePP, 0, sizeof toquePP);
#endif
}

static void cancelarAutomatico(void) {
  fecharEm = 0;
  if (pedAutomatico) {
    unattendedCarry=0;watchedAutomatically=0;lastAutoCounted[0]=0;
    pedT = pedE = 0;
    pedAutomatico = 0;
    if (temProximoJogavel()) {
      visivel = 1;
      dispensado = 0;
    }
  }
}

// Busca explicita revoga tambem um Next que o roteador ja estava aguardando.
// O pulo automatico de creditos usa so aguardar_busca e conserva o buffer.
void posplay_busca_usuario(void) {
  unattendedCarry=0;
  watchedAutomatically=0;stillWatchingPrompt=0;lastAutoCounted[0]=0;
  cancelarAutomatico();
  if (automaticoEntregue && temProximoJogavel()) {
    visivel = 1;
    dispensado = 0;
  }
  automaticoEntregue = 0;
  cancelarPreparo(0);
  proximopreparo_cancelar();
  preTentou = preVideoTentou = preVideoRenovou = 0;
  aguardaBusca = 1;
}

void posplay_troca_manual(void) {
  unattendedCarry=0;
  watchedAutomatically=0;stillWatchingPrompt=0;lastAutoCounted[0]=0;
  cancelarPreparo(0);
  pedT = pedE = 0;
  pedAutomatico = automaticoEntregue = 0;
  visivel = 0; fecharEm = 0; foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f; dispensado = 1;
  // Nao chamar proximopreparo_cancelar: consumir/adotar vem antes de fechar
  // o player. Um pedido de Anterior cancela o alvo diferente no roteador.
}

// Recuar e pedir para ficar neste episodio. O cartao continua servindo ao OK,
// mas a contagem nao volta enquanto a pessoa ainda esta na mesma janela.
void posplay_recuar(void) {
  unattendedCarry=0;watchedAutomatically=0;lastAutoCounted[0]=0;
  cancelarAutomatico();
  proximopreparo_cancelar();
  recuou = 1;
}

// A posicao da barra pode mostrar o destino antes de o seek chegar ao motor.
// Enquanto o player espera progresso real, Next continua sendo uma escolha.
void posplay_aguardar_busca(int aguardar) {
  aguardaBusca = aguardar != 0;
  if (aguardaBusca) cancelarAutomatico();
}

int posplay_pediu_episodio(int *t, int *e) {
  if (!temPedidoEpisodio()) return 0;
  if (t) *t = pedT;
  if (e) *e = pedE;
  pedT = pedE = 0;
  automaticoEntregue = pedAutomatico;
  pedAutomatico = 0;
  return 1;
}
int posplay_pediu_titulo(void) { int v = pedTitulo; pedTitulo = -1; return v; }

// Abertura A PEDIDO, pelo botao do player. Existe porque dispensar passou a
// grudar: sem uma porta de volta, quem apertasse Voltar uma vez nao veria mais
// os relacionados naquele filme. Limpa a dispensa de proposito — o pedido
// explicito vale mais que a recusa anterior.
// O indice que o player manda e o CORRENTE (idxAtual); guarda-se junto o id,
// para o desenho conferir (cat_indice_vivo) se uma troca de bloco caiu entre
// a atualizacao e ele.
static void fixarTitulo(int idxCatalogo) {
  const CatItem *ci = cat_item(idxCatalogo);
  idx = idxCatalogo;
  snprintf(idTitulo, sizeof idTitulo, "%s", ci ? ci->imdb : "");
}

int posplay_indice(void) { return cat_indice_vivo(idx, idTitulo); }

int posplay_abrir_relacionados(int idxCatalogo) {
  if (extras_n_relacionados() <= 0) return 0;
  fixarTitulo(idxCatalogo);
  serie = 0;
  foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f;
  fecharEm = 0;
  dispensado = 0;
  visivel = 1;
  return 1;
}

// O episodio SEGUINTE ao que esta tocando, na lista unica (ja ordenada por
// temporada e episodio). Devolve 0 quando o que toca e o ultimo.
// 2.0.3: a regra e prox_indice_seguinte, a MESMA de player_proximo_episodio
// (que abre a janela). Era "o indice seguinte ao do episodio tocando": com o
// episodio repetido na lista oferecia ele mesmo, e com o episodio fora da lista
// nao oferecia nada enquanto a janela ja estava aberta (e a legenda, subida).
static int acharProximo(int idxItem, int t, int e) {
  int n = cat_n_episodios(idxItem), i;
  const CatEp *base = n > 0 ? cat_episodio(idxItem, 0) : NULL, *px;
  if (!base) return 0;
  i = prox_indice_seguinte(base, n, t, e);
  px = i >= 0 ? cat_episodio(idxItem, i) : NULL;
  if (!px) return 0;
  proxT = px->temporada; proxE = px->episodio;
  snprintf(proxNome, sizeof proxNome, "%s", px->nome);
  proxFuturo = posplay_episodio_futuro(px, (long long)time(NULL) * 1000LL);
  return 1;
}

// So o gatilho e novo: identidade pelo catalogo, janela pelo player, consulta
// e fio pelo fontecache. Tudo que le catalogo/conta/perfil fica no fio da UI.
static void prepararProximo(int ehSerie, double resta) {
  FontecacheEscopo escopo;
  const CatEp *px;
  const CatItem *ci = cat_item(idx);
  long long quando;
  int t = 0, e = 0, n = cat_n_episodios(idx), proximo;
  proximopreparo_passo(preEpisodio);
  if (!ehSerie || !ci || jfid_e(ci->imdb)) {
    proximopreparo_cancelar(); return;
  }
  addons_capturar_escopo(&escopo);
  int video = ajustes_fonte_precarregar() && !ajustes_fonte_manual();
  if ((!ajustes_fonte_preparar() && !video) || ((preTentou || preVideoTentou) &&
      (escopo.geracao != preEscopo.geracao || escopo.addons != preEscopo.addons ||
       escopo.perfil != preEscopo.perfil || escopo.sessao != preEscopo.sessao ||
       strcmp(escopo.conta, preEscopo.conta)))) {
    cancelarPreparo(1);
    proximopreparo_cancelar();
    return;
  }
  // Uma renovacao, 10 s antes da contagem final: o preparo feito antes dos
  // creditos pode ter expirado. Nao consulta periodicamente durante o episodio.
  // O video ganha um minuto antes da janela para consultar os scrapers e
  // carregar amostras; o preparo antigo de metadados conserva os seus 10 s.
  if ((!video && (preTentou == 2 || (preTentou && resta > ajustes_proximo_contagem() + 10.0))) ||
      dispensado || recuou || aguardaBusca || durEstavel < PP_DUR_ESTAVEL_S ||
      !player_janela_proximo(video ? 60.0 : 10.0)) return;
  player_episodio_atual(&t, &e);
  if (t < 0 || e <= 0) return;
  proximo = prox_indice_seguinte(n ? cat_episodio(idx, 0) : NULL, n, t, e);
  px = proximo >= 0 ? cat_episodio(idx, proximo) : NULL;
  if (!px) return;
  quando = prox_episodio_data_ms(px);
  // Desconhecido nao prova que ja estreou: nao gasta rede especulativa.
  if (quando == PROX_SEM_DATA || quando > (long long)time(NULL) * 1000) return;
  if (!cat_id_stream(idx, px->temporada, px->episodio, preAlvo, sizeof preAlvo) ||
      jfid_e(preAlvo)) return;
  preEscopo = escopo;
  if (video) {
    // O novo toggle nao depende de preparar as fontes na pagina de titulo.
    // Usa lista/resolvedor proprios, sem substituir streams do episodio atual.
    if (!preVideoTentou) {
      preVideoTentou = 1;
      proximopreparo_agendar(idx, px->temporada, px->episodio,
                             preEpisodio, preAlvo, &preEscopo);
    } else if (!preVideoRenovou && resta <= ajustes_proximo_contagem() + 10.0) {
      if (proximopreparo_renovar(preEpisodio, preAlvo)) preVideoRenovou = 1;
    }
    return;
  }
  if (preTentou) {
    Stream *l = NULL;
    Uint32 idade = 0;
    int n;
    int acerto = fontecache_vod_pegar(preAlvo, "series", "", &preEscopo, &l, &n, &idade);
    free(l);
    if (acerto == FC_ACERTO && idade + fmax(resta, 0.0) * 1000 < FONTECACHE_VOD_VALIDADE_MS) {
      preTentou = 2;
      return;
    }
  }
  if (fontecache_precarregar_proximo(preAlvo, &preEscopo)) preTentou++;
}

// Marcador de creditos que o FILME aceita. Ver posplay_regra_filme.
// 2.0.3: a parte final do FILME e a de intro_creditos_janela_filme (12%,
// 5-15 min), a mesma do botao de pular creditos; era o ultimo quarto.
static int credAceito(double durSeg, double cred) {
  return cred > 1.0 && cred < durSeg && durSeg - cred <= intro_creditos_janela_filme(durSeg);
}

// A REGRA DO FILME, sem estado (o teste chama direto).
//
// #115, Owlphibia29: "More like this aparece cedo demais — no comeco do
// filme". A estimativa sem marcador ja nao subia no segundo zero (teto de
// metade da janela, 10/09), mas o MARCADOR passava sem conferencia nenhuma:
// `posSeg >= creditos` bastava. E os dois marcadores podem apontar para o
// comeco do filme:
//   - capitulo do Matroska: mkv_creditos_nomeados casava o PRIMEIRO nome com
//     "credit", e "Opening Credits" aos 90 s e capitulo comum em remux;
//   - TheIntroDB: dado de terceiro, e intro_creditos_seg devolvia o PRIMEIRO
//     trecho de creditos (um filme pode ter o de abertura e o final).
// Com qualquer um dos dois, o painel subia aos 90 s de filme. O cartao de
// proximo episodio tinha sanidade para isso desde o #34 (credJanela, no
// player.c); o de filme nunca teve.
//
// Agora, em ordem:
//   1. NUNCA ANTES DA METADE da duracao — vale para marcador e estimativa.
//   2. Marcador so vale no ULTIMO QUARTO (mesma regra que video_creditos ja
//      usa para o capitulo sem nome). Fora disso e recusado e cai na
//      estimativa, em vez de calar o painel para sempre.
//   3. Marcador aceito manda, e so ele (antes dele, a estimativa nao sobe).
//   4. Sem marcador, a estimativa proporcional de sempre.
int posplay_regra_filme(double posSeg, double durSeg, double creditosSeg) {
  double janela, resta;
  if (durSeg <= 1.0 || !ajustes_posplay_recomendacoes()) return 0;
  if (ajustes_posplay_percent()) return nv_settings_threshold(posSeg,durSeg,ajustes_posplay_percent());
  if (posSeg < durSeg * PP_FILME_METADE) return 0;
  if (credAceito(durSeg, creditosSeg)) return posSeg >= creditosSeg;
  // FILME SEM MARCADOR (MP4 nao tem capitulo; o TheIntroDB nao tem todo
  // filme): tempo FIXO antes do fim, intro_fim_estimado_filme — 3 min, 90 s
  // em filme de menos de 1 h, nada abaixo de 10 min. Decisao do dono em
  // 07/10/2026: nada de porcentagem (era 4,5% da duracao, entre 150 e 330 s).
  janela = intro_fim_estimado_filme(durSeg);
  if (janela <= 0.0) return 0;
  resta = durSeg - posSeg;
  return resta > 0.0 && resta <= janela;
}

// A CONTAGEM DO PROXIMO EPISODIO, sem estado (o teste chama direto). Devolve o
// novo `fecharEm` (0 = sem contagem). Regra (#470):
//   - autoplay desligado: nunca conta, o cartao espera a pessoa;
//   - ja contando: mantem o instante (cancelar e esconder o cartao, que zera);
//   - cartao apareceu: arma contagemSeg agora, limitada ao que resta.
// `restaReal` e o que falta do episodio em segundos de relogio.
Uint32 posplay_contagem(Uint32 agora, Uint32 fecharEm, double restaReal, int autoplay, int contagemSeg) {
  Uint32 alvo = 0;
  int armou = 0;
  if (!autoplay) return 0;
  if (fecharEm) return fecharEm;
  if (contagemSeg < 1) contagemSeg = 1;
  if (contagemSeg > 60) contagemSeg = 60;
  if (restaReal <= (double)contagemSeg) {
    if (restaReal < 0.0) restaReal = 0.0;
    alvo = agora + (Uint32)(restaReal * 1000.0); armou = 1;
  } else {
    alvo = agora + (Uint32)contagemSeg * 1000u; armou = 1;
  }
  if (armou && !alvo) alvo = 1;   // 0 quer dizer "sem contagem"
  return alvo;
}

// Backends can finish a fraction of a second short of duration. Trust EOF
// only near the real end, never an early/truncated stream or stale marker.
static int filmeNoFimConfirmado(double posSeg,double durSeg) {
  return durSeg>1.0 && video_terminou() && posSeg>=durSeg-PP_FIM_TOL_S;
}

void posplay_atualizar(float dt, Uint32 agora, double posSeg, double durSeg,
                       int ehSerie, int idxCatalogo, int janelaSerie) {
  // ◀ / ▶ crossfade (film page) and the receded window, re-sent each frame
  // by the player while the video is receded (posplay_fundo).
  if (relTroca < 1.0f) { relTroca += dt / 0.32f; if (relTroca >= 1.0f) { relTroca = 1.0f; relAnterior = -1; } }
  janelaReal.w = 0.0f;
  int interval=ajustes_ainda_episodios();
  if(interval!=watchedInterval){unattendedCarry=0;watchedInterval=interval;watchedAutomatically=0;stillWatchingPrompt=0;lastAutoCounted[0]=0;}
  if(!nv_perfil_sessao_atual(watchScope)){unattendedCarry=0;watchScope=nv_perfil_sessao_ler();watchedAutomatically=0;stillWatchingPrompt=0;lastAutoCounted[0]=0;}

  if(unattendedCarry){
    const CatItem *next=cat_item(idxCatalogo);int t=0,e=0;player_episodio_atual(&t,&e);
    if(!(t>=0&&e>0)&&next){t=next->temporada;e=next->episodio;}
    if(!next||strcmp(next->imdb,watchedNextTitle)||
       ((t!=watchedFromT||e!=watchedFromE)&&(t!=watchedNextT||e!=watchedNextE))){
      unattendedCarry=0;watchedAutomatically=0;lastAutoCounted[0]=0;
    }else if(t==watchedNextT&&e==watchedNextE)unattendedCarry=0;
  }
  int deveAparecer = 0;
  // DUAS FONTES DE MARCADOR, nesta ordem e por este motivo: o capitulo do
  // Matroska vem do PROPRIO arquivo que esta tocando, entao ele descreve esta
  // copia; o TheIntroDB descreve o LANCAMENTO, e uma copia com abertura
  // diferente ou com anuncio na frente sai deslocada. Quando o arquivo diz,
  // ele ganha.
  //
  // O segundo cobre o caso que antes so tinha estimativa: filme em MP4, que nao
  // tem capitulo nenhum para ler.
  double creditosSeg = video_creditos();
  if (creditosSeg <= 1.0) creditosSeg = intro_creditos_seg();
  anim = anim_mola(anim, visivel ? 1.0f : 0.0f, dt, NV_MOLA_TELA);
  // A CADA QUADRO, e nao so na abertura do painel (#190): com o cartao no ar o
  // `idx` guardado envelhecia junto com o catalogo.
  { char episodio[64] = "";
    int t = 0, e = 0;
    player_episodio_atual(&t, &e);
    if (ehSerie && t >= 0 && e > 0)
      cat_id_stream(idxCatalogo, t, e, episodio, sizeof episodio);
    if (preEpisodio[0] && strcmp(preEpisodio, episodio)) {
      cancelarPreparo(1);
      posplay_fechar();
    }
    snprintf(preEpisodio, sizeof preEpisodio, "%s", episodio);
  }
  fixarTitulo(idxCatalogo);
  if (durSeg - durVista > 2.0 || durVista - durSeg > 2.0) {
    durVista = durSeg;
    durEstavel = 0.0;
  } else {
    durEstavel += dt;
  }
  prepararProximo(ehSerie, vel_tempo_real(durSeg - posSeg, player_velocidade_efetiva()));
  if (durSeg <= 1.0) return;

  if (ehSerie && !janelaSerie && !aguardaBusca && recuou) {
    recuou = 0;
    preVideoTentou = preVideoRenovou = 0;
  }

  if (ehSerie) {
    // A JANELA quem decide e o player: ele e o unico que sabe se os creditos
    // comecaram (intro_ativo/INTRO_CREDITOS) alem dos dois minutos finais. Era
    // a mesma condicao que a caixa antiga de "Próximo episódio" usava, e passar
    // a usa-la aqui foi o que permitiu apagar aquela caixa — havia DUAS
    // interfaces de proximo episodio na mesma tela, uma feia e uma nova.
    //
    // A contagem comeca com o cartao se autoplay estiver ligado.
    deveAparecer = janelaSerie && durEstavel >= PP_DUR_ESTAVEL_S;
  } else {
    // FILME: a regra inteira mora em posplay_regra_filme, logo acima, que o
    // teste chama sem player nem rede. Aqui entra so a DURACAO ESTAVEL — ver
    // durEstavel.
    int fimConfirmado=filmeNoFimConfirmado(posSeg,durSeg);
    deveAparecer = ajustes_posplay_recomendacoes() && (durEstavel >= PP_DUR_ESTAVEL_S || fimConfirmado) &&
                   (posplay_regra_filme(posSeg, durSeg, creditosSeg) ||
                    (ajustes_posplay_percent()==100 && fimConfirmado) ||
                    (cat_item(idx) && cat_item(idx)->posCreditos && durSeg - posSeg <= PP_FIM_TOL_S));
  }

  // Saiu da zona (o dono voltou o filme): a dispensa perde a validade e o
  // painel pode subir de novo quando ele chegar ao fim outra vez.
  if (!deveAparecer) { dispensado = 0; movieRecommendationState=-1; }
  { const CatItem *ci = cat_item(idx);
    cenaNoFim = ci && ci->posCreditos && durSeg - posSeg <= PP_FIM_TOL_S;
    aguardaCena = ci && ci->posCreditos && !cenaNoFim;
    if (cenaNoFim && cenaPendente) { cenaPendente = 0; dispensado = 0; }
  }

  if (deveAparecer && !visivel && !dispensado) {
    if (!ehSerie) {
      int state=extras_n_relacionados()>0 ? 1 : extras_carregando() ? 2 : 0;
      // A missing list must not flood logcat every frame and erase the
      // metadata failure that explains it. Log state transitions only.
      if(state!=movieRecommendationState) {
        printf("[posplay] movie recommendation window: related=%d loading=%d\n",
               extras_n_relacionados(),state==2);
        movieRecommendationState=state;
      }
    }
    serie = ehSerie;
    foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f;
    proxT = proxE = 0; proxNome[0] = 0; proxFuturo = 0;
    if (serie) {
      int t = 0, e = 0;
      const CatItem *ci = cat_item(idx);
      // QUAL EPISODIO ESTA TOCANDO: pergunta ao PLAYER, e nao ao item do
      // catalogo.
      //
      // MEDIDO NA TV, e so por isso encontrado: tocando Silo T2E8, o painel
      // ofereceu T2E7 e o log confirmou ("automatico (verificado): Silo S02
      // E07"). O item ainda apontava para T2E6 — foi por ele que a serie
      // entrou, pela fileira "Continuar assistindo" — e nada o move quando a
      // reproducao comeca pelo botao da pagina de titulo. "O proximo de E6" e
      // E7, e era literalmente isso que o codigo pedia.
      //
      // epT/epE do player sao a unica fonte que acompanha a reproducao de
      // verdade: player_definir_episodio os escreve em todo caminho que abre
      // um episodio. O item continua valendo de reserva para o caso de o
      // player nao ter episodio nenhum (idx trocado, dado incompleto).
      player_episodio_atual(&t, &e);
      if (!(t >= 0 && e > 0) && ci) { t = ci->temporada; e = ci->episodio; }
      if (t >= 0 && e > 0 && acharProximo(idx, t, e)) {
        fecharEm = 0;            // arma neste quadro, depois de abrir o cartao
        visivel = 1;
      }
      // Sem proximo episodio (fim da serie): nao aparece nada. Mostrar um
      // painel vazio no ultimo episodio seria pior que nao mostrar.
    } else if (extras_n_relacionados() > 0) {
      fecharEm = 0;              // filme nao tem contagem: o dono escolhe
      visivel = 1;
    }
  }

  // Aviso de estreia no fim real do video: sai do caminho e o player encerra
  // como no ultimo episodio (volta para a pagina do titulo).
  if (visivel && serie && !aguardaCena && proxFuturo && filmeNoFimConfirmado(posSeg, durSeg))
    esconder();

  // Arma ao abrir o cartao. Desligado, espera OK.
  // Recuo e busca pendente suspendem a mesma contagem configuravel.
  if (visivel && serie && janelaSerie && !recuou && !aguardaBusca) {
    // #202: a 1,5x os segundos do arquivo passam mais depressa que os do relogio.
    double resta = vel_tempo_real(durSeg - posSeg, player_velocidade_efetiva());
    // Episodio que ainda nao foi ao ar nao tem o que tocar: nada de contagem.
    if (aguardaCena || stillWatchingPrompt || proxFuturo) fecharEm = 0;
    else fecharEm = posplay_contagem(agora, fecharEm, resta, ajustes_proximo_auto(), ajustes_proximo_contagem());
  }

  // A contagem so vale para o proximo episodio.
  if (visivel && fecharEm && agora >= fecharEm) {
    char episodeKey[256];snprintf(episodeKey,sizeof episodeKey,"%s|%s",idTitulo,preEpisodio);
    unsigned increment=strcmp(lastAutoCounted,episodeKey)!=0;
    if (nv_settings_still_due(watchedAutomatically+increment,ajustes_ainda_episodios())) {
      stillWatchingPrompt=1;fecharEm=0;
    } else {
      watchedAutomatically+=increment;snprintf(lastAutoCounted,sizeof lastAutoCounted,"%s",episodeKey);
      player_episodio_atual(&watchedFromT,&watchedFromE);
      if(!(watchedFromT>=0&&watchedFromE>0)){const CatItem *ci=cat_item(idx);if(ci){watchedFromT=ci->temporada;watchedFromE=ci->episodio;}}
      watchedNextT=proxT;watchedNextE=proxE;snprintf(watchedNextTitle,sizeof watchedNextTitle,"%s",idTitulo);
      unattendedCarry=1;pedT = proxT; pedE = proxE;
      pedAutomatico = 1;esconder();
    }
  }
}

int posplay_evento(const SDL_Event *e) {
  int k;
  if (!visivel || e->type != SDL_KEYDOWN) return 0;
#ifdef NV_TOUCH_UI
  if (toquerol_navegacao(e)) toquerol_limpar(&toquePP);
#endif
  k = e->key.keysym.sym;
  unattendedCarry=0;
  if (stillWatchingPrompt) {
    // A held remote OK must never confirm a newly displayed presence prompt.
    if (e->key.repeat && (k == SDLK_RETURN || k == SDLK_KP_ENTER)) return 1;
    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) { stillWatchingPrompt=0;watchedAutomatically=0; }
    else if (k != SDLK_AC_BACK && k != SDLK_ESCAPE && k != SDLK_BACKSPACE && k != SDLK_DOWN) return 1;
  } else watchedAutomatically=0; // Explicit interaction breaks an unattended sequence.
  if (k == SDLK_AC_BACK || k == SDLK_ESCAPE || k == SDLK_BACKSPACE ||
      e->key.keysym.scancode == NV_SCANCODE_BACK) {
    // Dispensar CANCELA a contagem e deixa o video terminar em paz. E GRUDA:
    // fechar sem marcar fazia o painel voltar no quadro seguinte. E esconder,
    // e nao posplay_fechar, justamente porque a marca tem de sobreviver — o
    // par "fechar; marcar de novo" que estava aqui era o mesmo tropeco que
    // apagava o pedido do OK logo abaixo.
    esconder();
    return 1;
  }
  // BAIXO tira o painel do caminho E devolve os controles. Pedido do dono: com
  // o painel no ar ele quer poder descer para a barra de tempo sem perder a
  // reproducao. Voltar apenas dispensa; BAIXO dispensa e mostra o player.
  // Film page (TV): Down from the video window comes back to the buttons, as
  // official's focusProperties; from the buttons it keeps the owner's exit.
  if (k == SDLK_DOWN && !serie && !aguardaCena && !telefoneui_ativo() && botaoFilme == PB_JANELA) {
    botaoFilme = PB_ABRIR;
    return 1;
  }
  if (k == SDLK_DOWN) {
    esconder();
    return 2;
  }
  // Cena pos-creditos (serie ou filme): OK = continuar assistindo, sem trocar
  // de titulo. O cartao inteiro volta no fim real do arquivo.
  // Serie com proximo: um segundo botao leva ao proximo episodio na mao.
  // Every other arrow stays with the card. Returning 0 here sent Left/Right to
  // the player, which SEEKS while its controls are hidden (#121): the video
  // jumped under the card and could leave the end window, closing it.
  if (aguardaCena) {
    int temProx = temProximoJogavel();
    if (temProx && k == SDLK_RIGHT && foco == 0) { foco = 1; return 1; }
    if (temProx && k == SDLK_LEFT && foco == 1)  { foco = 0; return 1; }
    if (k == SDLK_LEFT || k == SDLK_RIGHT || k == SDLK_UP) return 1;
    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
      if (temProx && foco == 1) { player_aprender_creditos(); pedT = proxT; pedE = proxE; }
      else cenaPendente = 1;
      esconder(); return 1;
    }
    return 0;
  }
  if (serie) {
    // Ainda nao foi ao ar: o OK so fecha o aviso, o episodio segue tocando.
    if (proxFuturo && (k == SDLK_RETURN || k == SDLK_KP_ENTER)) { esconder(); return 1; }
    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
      player_aprender_creditos(); pedT = proxT; pedE = proxE; pedAutomatico = 0; esconder(); return 1;
    }
    if (k == SDLK_LEFT || k == SDLK_RIGHT || k == SDLK_UP) return 1;
    return 0;
  }
  { int n = nRel();
    if (telefoneui_ativo()) {
      if (k == SDLK_RIGHT && foco + 1 < n) { foco++; return 1; }
      if (k == SDLK_LEFT  && foco > 0)     { foco--; return 1; }
      if (k == SDLK_LEFT || k == SDLK_RIGHT || k == SDLK_UP) return 1;
    } else {
      // Official focus order: [Details] [<] [>], Up = the video window. A
      // disabled arrow is skipped, as Compose skips a disabled IconButton.
      int antOk = n > 1 && foco > 0, proxOk = n > 1 && foco + 1 < n;
      if (botaoFilme == PB_JANELA) {
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER) { esconder(); return 2; }   // return to player
        if (k == SDLK_LEFT || k == SDLK_RIGHT || k == SDLK_UP) return 1;
      }
      if (k == SDLK_UP) { botaoFilme = PB_JANELA; return 1; }
      if (k == SDLK_RIGHT) {
        if (botaoFilme == PB_ABRIR) botaoFilme = antOk ? PB_ANT : proxOk ? PB_PROX : PB_ABRIR;
        else if (botaoFilme == PB_ANT && proxOk) botaoFilme = PB_PROX;
        return 1;
      }
      if (k == SDLK_LEFT) {
        if (botaoFilme == PB_PROX) botaoFilme = antOk ? PB_ANT : PB_ABRIR;
        else if (botaoFilme == PB_ANT) botaoFilme = PB_ABRIR;
        return 1;
      }
      if ((k == SDLK_RETURN || k == SDLK_KP_ENTER) && (botaoFilme == PB_ANT || botaoFilme == PB_PROX)) {
        int d = botaoFilme == PB_ANT ? -1 : 1;
        if (foco + d >= 0 && foco + d < n) {
          relAnterior = foco; relTroca = 0.0f; foco += d;
          postplay_preview_close();
        }
        // Official: focus stays on the arrow just used while it still works.
        antOk = foco > 0; proxOk = foco + 1 < n;
        if (botaoFilme == PB_ANT && !antOk) botaoFilme = proxOk ? PB_PROX : PB_ABRIR;
        if (botaoFilme == PB_PROX && !proxOk) botaoFilme = antOk ? PB_ANT : PB_ABRIR;
        return 1;
      }
    }
    if (foco >= n) foco = n > 0 ? n - 1 : 0;
    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
      const char *id = extras_relacionado_imdb(foco);
      int alvo = id[0] ? cat_indice_por_imdb(id) : -1;
      if (alvo >= 0) { pedTitulo = alvo; esconder(); }
      // NAO ESTA NO CATALOGO LOCAL, que e o caso NORMAL: o relacionado vem do
      // Trakt/TMDB. Mesmo caminho do detalhe (detail.c, desc_pedir_titulo_semente):
      // "tmdb:<n>" (recomendacao do TMDB, sem imdb) abre pelo id do TMDB —
      // desc_pedir_titulo cortava no ':' e buscava o id "tmdb", o OK morria mudo.
      else if (!strncmp(id, "tmdb:", 5))
        { desc_pedir_titulo_semente("", atol(id + 5), "movie", extras_relacionado_titulo(foco),
                                    extras_relacionado_ano(foco), extras_relacionado_poster(foco));
          esconder(); }
      else if (id[0])
        { desc_pedir_titulo_semente(id, 0, "movie", extras_relacionado_titulo(foco),
                                    extras_relacionado_ano(foco), extras_relacionado_poster(foco));
          esconder(); }
      return 1;
    } }
  return 0;
}

// #177: a mesma regra da lista de episodios (#133). O proximo episodio e, por
// definicao, o que a pessoa ainda nao viu: fica desfocado enquanto o ajuste
// estiver ligado, salvo se o mapa afirma que ja foi visto (reassistindo).
int posplay_desfocar_thumb(int idxCatalogo, int temporada, int episodio) {
  const CatItem *ci;
  if (!ajustes_desfocar_nao_assistidos()) return 0;
  ci = cat_item(idxCatalogo);
  return !(ci && vistoep_estado(ci->imdb, temporada, episodio) == 1);
}

// OFFICIAL NUVIO TV LAYOUT (owner, 12/11: "so it matches Nuvio's one").
//
// FILM — PostPlayRecommendationOverlay.kt. One recommendation at a time: its
// backdrop fills the screen (posplay_fundo, drawn by the player around the
// video hole), the video recedes to a 16:9 window 32% wide in the top-right
// corner (player.c, destinoComRecuo), and a column at the bottom-left reads
// "Because you watched X", the logo (or title), genres · year · runtime ·
// rating, three lines of synopsis, then the buttons: Details and the ◀ ▶ pair
// that steps through the list. Up focuses the video window (OK returns to the
// player); Down still dismisses and shows the controls (owner's shortcut).
// Our Details opens the title page; official's Play starts the film directly,
// which this port has no route for from inside the player.
//
// EPISODE — PostPlayOverlay.kt. A compact card at the bottom-right: still,
// "Next episode", "S1 E4 • Name", the countdown line and a Play pill. The same
// card carries the post-credits choice and "Are you still watching?".
//
// Measures are the official dp at 2x (Android TV 1080p = 960x540 dp).
//
// POINTER (#99). The panel sits over the player's controls (which stop
// registering while it is up): a full-screen GUARD absorbs clicks on empty
// space — without it any click would become OK and start the next episode.
// Hover only moves focus; the episode starts only from a click on its button.
int posplay_teste_botao(void) { return botaoFilme; }

static void ponteiroComecar(int a, int b) { (void)a; (void)b; }
static void ponteiroCena(int i, int b) { (void)b; if (visivel && aguardaCena && temProximoJogavel()) foco = i; }
static void ponteiroBotao(int i, int b) { (void)b; if (visivel && !serie && !aguardaCena) botaoFilme = i; }
static void ponteiroCartaz(int i, int b) {
  int n = nRel();
  (void)b;
  if (!visivel || serie || i < 0 || i >= n || foco == i) return;
  foco = i;
#ifdef NV_TOUCH_UI
  toquerol_limpar(&toquePP);
#endif
}
int posplay_teste_foco(void) { return foco; }

// The film's focus ring: accent outline 2 dp outside the shape.
static void anelFoco(GfxRect r, float raio, float a) {
  float ar, ag, ab;
  if (a <= 0.002f) return;
  ajustes_acento(&ar, &ag, &ab);
  gfx_anel((GfxRect){ r.x - 6.0f, r.y - 6.0f, r.w + 12.0f, r.h + 12.0f }, raio, 4.0f, ar, ag, ab, a);
}

// Official PostPlayRecommendationButton: primary = white with black ink in both
// states (the ring marks focus); secondary = dark card, accent when focused.
static void botaoPP(GfxRect r, const char *rotulo, const char *icone, int primario, int sel, float a) {
  float ar, ag, ab, ink;
  ajustes_acento(&ar, &ag, &ab);
  if (primario) { gfx_cor(r, 0.5f, 1, 1, 1, a); ink = 0.0f; }
  else if (sel) { gfx_cor(r, 0.5f, ar, ag, ab, a); ink = plrui_tinta() / 255.0f; }
  else { gfx_cor(r, 0.5f, 0.11f, 0.11f, 0.12f, 0.86f * a); ink = 0.95f; }
  if (sel) anelFoco(r, 0.5f, a);
  { TxtLinha l = txt_linha(TXT_G28B, rotulo, (int)(ink * 255), (int)(ink * 255), (int)(ink * 255), 255);
    float iw = icone ? 36.0f + 16.0f : 0.0f, x0 = r.x + (r.w - iw - (float)l.w) * 0.5f;
    if (icone) gfx_icone((GfxRect){ x0, r.y + (r.h - 36.0f) * 0.5f, 36.0f, 36.0f }, icone, ink, ink, ink, a);
    txt_desenhar_alpha(l, x0 + iw, r.y + (r.h - (float)l.h) * 0.5f, a); }
}
static void setaPP(GfxRect r, const char *icone, int ativa, int sel, float a) {
  float ar, ag, ab, ink = sel ? plrui_tinta() / 255.0f : 0.95f, k = ativa ? 1.0f : 0.38f;
  ajustes_acento(&ar, &ag, &ab);
  if (sel) gfx_cor(r, 0.5f, ar, ag, ab, a);
  else gfx_cor(r, 0.5f, 0.11f, 0.11f, 0.12f, 0.86f * a * (ativa ? 1.0f : 0.7f));
  if (sel) anelFoco(r, 0.5f, a);
  gfx_icone((GfxRect){ r.x + (r.w - 48.0f) * 0.5f, r.y + (r.h - 48.0f) * 0.5f, 48.0f, 48.0f },
            icone, ink, ink, ink, a * k);
}


// Backdrop + official scrims, behind everything. Called by the player in REAL
// pixels around the receded video (player_fundo_fora_do_furo). 1 = drew.
int posplay_fundo(GfxRect tela, GfxRect furo) {
  int i = foco, n = nRel();
  if (!visivel && anim < 0.01f) return 0;
  if (serie || aguardaCena || telefoneui_ativo() || n <= 0 || i < 0 || i >= n) return 0;
  janelaReal = furo;
  gfx_cor(tela, 0.0f, 0.043f, 0.047f, 0.055f, 1.0f);
  { int k;
    for (k = 0; k < 2; k++) {
      int j = k == 0 ? relAnterior : i;
      float al = k == 0 ? 1.0f : (relAnterior >= 0 ? relTroca : 1.0f);
      const char *fu = j >= 0 && j < n ? extras_relacionado_fundo(j) : "";
      const char *po = j >= 0 && j < n ? extras_relacionado_poster(j) : "";
      const char *arte = fu[0] ? fu : po;
      GLuint t = arte[0] ? tex_obter_larg(arte, tela.w) : 0;
      if (k == 0 && (relAnterior < 0 || relTroca >= 1.0f)) continue;
      if (t) {
        gfx_tex_aspect_atual = tex_aspecto(arte);
        gfx_rect(tela, t, GFX_CARD, 0, 0, 0, 0.0f, 0, 0, 0, al);
        gfx_tex_aspect_atual = 0.0f;
      }
    } }
  // Horizontal 0.88 -> 0.16 (54%) -> 0.22, vertical 0.08 -> 0 (58%) -> 0.82.
  gfx_veu_css_base(tela, 2, 0.0f, 0.62f, 0.70f, 0.18f);
  gfx_veu_css(tela, 0, 0.0f, 0.46f, 0.80f);
  gfx_rect((GfxRect){ furo.x - 40.0f, furo.y - 10.0f, furo.w + 80.0f, furo.h + 90.0f }, 0, GFX_SOMBRA,
           1.0f, 0, 0, 0.5f, 0, 0, 0, 0.45f);
  return 1;
}

static void desenharFilmeTV(Uint32 agora, float a);
static void desenharCartaoEpisodio(Uint32 agora, float a);
static void desenharFilmeTelefone(Uint32 agora, float a);

static void posplay_desenharCorpo_(Uint32 agora, float baseY);
// Camada ampliada (escala.h): o corpo desenha na tela virtual.
void posplay_desenhar(Uint32 agora, float baseY) {
  ESCALA_INI();
  posplay_desenharCorpo_(agora, baseY);
  ESCALA_FIM();
}
#ifdef NV_SHOT_HOOKS
static int shotEstado, shotCena, shotAinda;   // posplay_shot_estado: survives posplay_atualizar
static GfxRect shotCartao;                     // the compact card's last rect (tests)
GfxRect posplay_shot_cartao(void) { return shotCartao; }
#endif
static void posplay_desenharCorpo_(Uint32 agora, float baseY) {
  float a = anim;
  (void)baseY;
#ifdef NV_SHOT_HOOKS
  if (shotEstado) { aguardaCena = shotCena; stillWatchingPrompt = shotAinda; }
#endif
  if(a<.99f||aguardaCena||stillWatchingPrompt||!ajustes_posplay_trailer())postplay_preview_close();
  if (a < 0.01f) return;
  if (visivel) ponteiro_alvo(0, 0, NV_TELA_W, NV_TELA_H, NULL, NULL, 0, 0);   // anteparo
  { int i = cat_indice_vivo(idx, idTitulo);
    if (i < 0) {postplay_preview_close();return;}
    idx = i; }
  if (serie || aguardaCena) { desenharCartaoEpisodio(agora, a); return; }
  if (telefoneui_ativo()) desenharFilmeTelefone(agora, a);
  else desenharFilmeTV(agora, a);
}

// --- EPISODE / POST-CREDITS / STILL WATCHING: the compact card --------------
static void pilulaPP(GfxRect r, const char *rotulo, int sel, int contorno, float a) {
  float ar, ag, ab, ink = sel ? plrui_tinta() / 255.0f : 1.0f;
  ajustes_acento(&ar, &ag, &ab);
  if (sel) gfx_cor(r, 0.5f, ar, ag, ab, a);
  else if (contorno) gfx_anel(r, 0.5f, 2.0f, 1, 1, 1, 0.22f * a);
  gfx_icone((GfxRect){ r.x + 20.0f, r.y + (r.h - 28.0f) * 0.5f, 28.0f, 28.0f }, "pl_play-f", ink, ink, ink, a);
  { TxtLinha l = txt_linha(TXT_G21B, rotulo, (int)(ink * 255), (int)(ink * 255), (int)(ink * 255), 255);
    txt_desenhar_alpha(l, r.x + 20.0f + 28.0f + 6.0f, r.y + (r.h - (float)l.h) * 0.5f, a); }
}
static float pilulaLarg(const char *rotulo) {
  TxtLinha l = txt_linha(TXT_G21B, rotulo, 255, 255, 255, 255);
  return 20.0f + 28.0f + 6.0f + (float)l.w + 22.0f;
}

static void desenharCartaoEpisodio(Uint32 agora, float a) {
  const CatItem *ci = cat_item(idx);
  const CatEp *px = NULL;
  int i, n = serie ? cat_n_episodios(idx) : 0;
  int resta = fecharEm > agora ? (int)((fecharEm - agora + 999) / 1000) : 0;
  int duas = aguardaCena && temProximoJogavel();
  // So o aviso de estreia: sem botao de tocar.
  int soAviso = proxFuturo && temProximo() && !aguardaCena && !stillWatchingPrompt;
  // Owner (12/11): bottom-LEFT, mirroring official's bottom-right card; it
  // slides in from the left. The player's skip button moves right while the
  // card is up (player.c), so the two never overlap.
  float sai = (1.0f - a) * 0.5f * PP_CARD_W2;
  GfxRect card, tr;
  char rotulo[96], titulo[200], status[96];
  const char *b0, *b1 = NULL;
  float pw0, pw1 = 0.0f, ty, tx, tw, pills;
  for (i = 0; i < n; i++) {
    const CatEp *e = cat_episodio(idx, i);
    if (e && e->temporada == proxT && e->episodio == proxE) { px = e; break; }
  }
  // Texts first: the card grows (to the left) when a language needs it.
  if (stillWatchingPrompt) snprintf(rotulo, sizeof rotulo, "%s", i18n("Você ainda está assistindo?"));
  else if (aguardaCena) snprintf(rotulo, sizeof rotulo, "%s", i18n("Cena pós-créditos"));
  else snprintf(rotulo, sizeof rotulo, "%s", i18n("Próximo episódio"));
  if (serie && temProximo()) {
    char cod[32];
    snprintf(cod, sizeof cod, i18n("T%dE%d"), proxT, proxE);
    { char *p = strchr(cod, 'E'); if (p && p > cod && p[-1] != ' ') {   // "S1 E4", official
        memmove(p + 1, p, strlen(p) + 1); *p = ' '; } }
    snprintf(titulo, sizeof titulo, "%s \xe2\x80\xa2 %s", cod, (px && px->nome[0]) ? px->nome : "");
    if (!(px && px->nome[0])) titulo[strlen(cod)] = 0;
  } else snprintf(titulo, sizeof titulo, "%s", ci ? ci->titulo : "");
  status[0] = 0;
  if (soAviso)
    ep_rotulo_estreia(px ? px->dataIso : "", (long long)time(NULL) * 1000LL, 0,
                      ajustes_idioma(), status, sizeof status);
  else if (!aguardaCena && !stillWatchingPrompt && resta > 0)
    snprintf(status, sizeof status, i18n("Reproduzindo em %d s"), resta);
  b0 = soAviso ? NULL : stillWatchingPrompt ? i18n("Reproduzir") : aguardaCena ? i18n("Seguir assistindo") : i18n("Reproduzir");
  if (duas) b1 = i18n("Próximo episódio");
  pw0 = b0 ? pilulaLarg(b0) : 0.0f; if (b1) pw1 = pilulaLarg(b1);
  pills = pw0 + (b1 ? 12.0f + pw1 : 0.0f);
  card.w = PP_CARD_W2; card.h = 18.0f * 2 + PP_THUMB_H2;
  { TxtLinha l = txt_linha(TXT_G28B, titulo, 255, 255, 255, 255);
    float preciso = 20.0f + PP_THUMB_W2 + 20.0f + fminf((float)l.w, 360.0f) + 16.0f + pills + 20.0f;
    if (preciso > card.w) card.w = fminf(preciso, NV_TELA_W - 2.0f * 52.0f); }
  // Two pills never squeeze the text: they go under it, the card grows down.
  if (duas && 20.0f + PP_THUMB_W2 + 20.0f + 240.0f + 16.0f + pills + 20.0f > card.w) card.h += 64.0f;
  card.x = 52.0f - sai;
  card.y = NV_TELA_H - 60.0f - card.h;
#ifdef NV_SHOT_HOOKS
  shotCartao = card;
#endif
  // Official 0xE3191919 card, 1 dp border at 16%, 14 dp corners. Glass keeps
  // the app's material under it.
  if (ajustes_vidro()) plrui_material_video(card, 28.0f, a);
  gfx_cor(card, 28.0f / card.h, 0.098f, 0.098f, 0.098f, 0.89f * a * (ajustes_vidro() ? 0.6f : 1.0f));
  gfx_anel(card, 28.0f / card.h, 2.0f, 1, 1, 1, 0.16f * a);
  if (!duas) anelFoco(card, 28.0f / card.h, a);   // the card is the focused control
  tr = (GfxRect){ card.x + 20.0f, card.y + 18.0f, PP_THUMB_W2, PP_THUMB_H2 };
  { const char *arte = (px && px->thumb[0]) ? px->thumb : (ci ? ci->backdrop : "");
    GLuint t = arte[0] ? tex_obter_larg(arte, tr.w) : 0;
    gfx_cor(tr, 18.0f / tr.h, 0.102f, 0.106f, 0.125f, a);
    // DESFOCAR NAO ASSISTIDOS (#177): o proximo e o que a pessoa nao viu.
    if (t && posplay_desfocar_thumb(idx, proxT, proxE))
      t = gfx_desfocado(t, arte);
    // Sem copia desfocada pronta, gfx_desfocado devolve 0 e fica o fundo.
    // A textura retornada ja esta desfocada: nunca a envie de novo ao cache.
    if (t) {
      gfx_tex_aspect_atual = tex_aspecto(arte);
      gfx_card_para(NV_CARD_EPISODES, tr, t, GFX_CARD, 0, 0, 0, 18.0f / tr.h, 0, 0, 0, a);
      gfx_tex_aspect_atual = 0.0f;
      gfx_veu_css(tr, 0, 0.0f, 1.0f, 0.32f * a);
    } }
  postplay_preview_draw(idTitulo,a>.99f&&ajustes_posplay_trailer()&&!stillWatchingPrompt&&!aguardaCena,ajustes_posplay_trailer_espera_ms(),agora,tr);
  tx = tr.x + tr.w + 20.0f;
  tw = card.x + card.w - 20.0f - tx - (card.h > 18.0f * 2 + PP_THUMB_H2 + 1.0f || !b0 ? 0.0f : pills + 16.0f);
  { TxtLinha l0 = txt_linha_corta(TXT_G22M, rotulo, 255, 255, 255, 204, tw);
    TxtLinha l1 = txt_linha_corta(TXT_G28B, titulo, 255, 255, 255, 255, tw);
    TxtLinha l2 = txt_linha_corta(TXT_G22M, status, 255, 255, 255, 199, tw);
    float alto = (float)l0.h + 4.0f + (float)l1.h + (status[0] ? 4.0f + (float)l2.h : 0.0f);
    ty = tr.y + (tr.h - alto) * 0.5f;
    txt_desenhar_alpha(l0, tx, ty, a); ty += (float)l0.h + 4.0f;
    txt_desenhar_alpha(l1, tx, ty, a); ty += (float)l1.h + 4.0f;
    if (status[0]) txt_desenhar_alpha(l2, tx, ty, a); }
  { float h = 56.0f, by, bx;
    if (card.h > 18.0f * 2 + PP_THUMB_H2 + 1.0f) { bx = tx; by = card.y + card.h - 18.0f - h; }
    else { bx = card.x + card.w - 20.0f - pills; by = card.y + (card.h - h) * 0.5f; }
    if (b0) pilulaPP((GfxRect){ bx, by, pw0, h }, b0, duas && foco == 0, 1, a);
    if (b1) pilulaPP((GfxRect){ bx + pw0 + 12.0f, by, pw1, h }, b1, foco == 1, 1, a);
    if (visivel && a > 0.99f && a < 1.01f) {
      if (duas) {
        ponteiro_alvo(bx, by, pw0, h, ponteiroCena, NULL, 0, 0);
        ponteiro_alvo(bx + pw0 + 12.0f, by, pw1, h, ponteiroCena, NULL, 1, 0);
      } else ponteiro_alvo(card.x, card.y, card.w, card.h, ponteiroComecar, NULL, 0, 0);
    } }
}

// --- FILM, TV: one recommendation over its backdrop --------------------------
static void notaPP(int i, float x, float yc, float a) {
  int fonte = 0, v = extras_relacionado_nota(i, &fonte);
  const char *marca = fonte == EX_TMDB ? "tmdb" : fonte == EX_TRAKT ? "trakt" : NULL;
  char num[16];
  float asp = fonte == EX_TMDB ? 1.40f : 1.0f, h = 30.0f;
  if (v < 0 || !marca) return;
  { const char *cam = extras_caminho_marca_nome(marca);
    GLuint t = tex_obter(cam);
    float ap = tex_aspecto(cam);
    float w = h * (ap > 0.0f ? ap : asp);
    if (t) gfx_rect((GfxRect){ x, yc - h * 0.5f, w, h }, t, GFX_TEXTO, 0, 0, 0, 0, 0.93f, 0.94f, 0.96f, a);
    x += w + 10.0f; }
  if (fonte == EX_TMDB) snprintf(num, sizeof num, "%d", v);
  else snprintf(num, sizeof num, "%d%%", v);
  { TxtLinha l = txt_linha(TXT_HERO_META_GG, num, 255, 255, 255, 178);
    txt_desenhar_alpha(l, x, yc - (float)l.h * 0.5f, a); }
}
static float notaLarg(int i) {
  int fonte = 0, v = extras_relacionado_nota(i, &fonte);
  if (v < 0 || (fonte != EX_TMDB && fonte != EX_TRAKT)) return 0.0f;
  { char num[16]; TxtLinha l;
    snprintf(num, sizeof num, fonte == EX_TMDB ? "%d" : "%d%%", v);
    l = txt_linha(TXT_HERO_META_GG, num, 255, 255, 255, 178);
    return 30.0f * (fonte == EX_TMDB ? 1.40f : 1.0f) + 10.0f + (float)l.w; }
}

static void desenharFilmeTV(Uint32 agora, float a) {
  int n = nRel(), i = foco;
  const CatItem *ci = cat_item(idx);
  float sobe = (1.0f - a) * 24.0f, at = a * (relTroca < 1.0f ? relTroca : 1.0f);
  float s = gfx_escala_ui() > 0.0f ? gfx_escala_ui() : 1.0f;
  // The window, in this layer's units; the column never runs under it.
  float jx = janelaReal.w > 0.0f ? janelaReal.x / s : NV_TELA_W * 0.66f;
  float colW = fminf(NV_TELA_W * 0.52f - PP_COL_X, jx - 32.0f - PP_COL_X);
  float x = PP_COL_X, y = NV_TELA_H - PP_COL_BASE + sobe, btW;
  if (n <= 0 || i < 0 || i >= n) { postplay_preview_close(); return; }
  // The receded video gets official's 1 dp border at 20% (and the ring on Up).
  if (janelaReal.w > 0.0f) {
    GfxRect j = { janelaReal.x / s, janelaReal.y / s, janelaReal.w / s, janelaReal.h / s };
    gfx_anel(j, 24.0f / j.h, 2.0f, 1, 1, 1, 0.20f * a);
    if (botaoFilme == PB_JANELA) anelFoco(j, 24.0f / j.h, a);
    if (visivel && a > 0.99f && a < 1.01f) ponteiro_alvo(j.x, j.y, j.w, j.h, ponteiroBotao, NULL, PB_JANELA, 0);
  }
  // Buttons, from the bottom up: Details | ◀ ▶.
  { int setas = n > 1;
    float disp = colW - (setas ? 2.0f * (PP_SETA + PP_VAO) : 0.0f);
    float by = y - PP_BT_H, bx = x;
    btW = fminf((colW - PP_VAO) * 0.5f, disp);
    botaoPP((GfxRect){ bx, by, btW, PP_BT_H }, i18n("Detalhes"), "pl_info", 1, botaoFilme == PB_ABRIR, a);
    if (visivel && a > 0.99f && a < 1.01f) ponteiro_alvo(bx, by, btW, PP_BT_H, ponteiroBotao, NULL, PB_ABRIR, 0);
    if (setas) {
      float sx = bx + btW + PP_VAO;
      setaPP((GfxRect){ sx, by, PP_SETA, PP_SETA }, "pl_chevron-left", i > 0, botaoFilme == PB_ANT, a);
      setaPP((GfxRect){ sx + PP_SETA + PP_VAO, by, PP_SETA, PP_SETA }, "pl_chevron-right", i + 1 < n,
             botaoFilme == PB_PROX, a);
      if (visivel && a > 0.99f && a < 1.01f) {
        if (i > 0) ponteiro_alvo(sx, by, PP_SETA, PP_SETA, ponteiroBotao, NULL, PB_ANT, 0);
        if (i + 1 < n) ponteiro_alvo(sx + PP_SETA + PP_VAO, by, PP_SETA, PP_SETA, ponteiroBotao, NULL, PB_PROX, 0);
      }
    }
    y = by - 48.0f; }   // xl 24 dp above the actions
  // Synopsis (3 lines), meta line, logo/title, "Because you watched".
  { const char *sin = extras_relacionado_sinopse(i);
    float lead = 40.0f;
    int linhas = sinopse_linhas(TXT_DET_SIN, lead, 3);
    if (sin[0]) {
      // Measure first (alpha 0 off-screen), then draw bottom-aligned.
      float h = sinopse_bloco_corta(TXT_DET_SIN, sin, 255, 255, 255, -10000.0f, 0.0f, colW * 0.92f, lead, 0.0f, linhas);
      if (h <= 0.0f) h = lead * (float)linhas;
      y -= h;
      sinopse_bloco_corta(TXT_DET_SIN, sin, 255, 255, 255, x, y, colW * 0.92f, lead, at * 0.80f, linhas);
      y -= 24.0f;
    } }
  { char meta[200], dur[24];
    const char *gen = extras_relacionado_generos(i), *ano = extras_relacionado_ano(i);
    int mins = extras_relacionado_duracao(i);
    float nw = notaLarg(i);
    dur[0] = 0;
    if (mins > 0) {
      if (mins >= 60) snprintf(dur, sizeof dur, "%dh %dm", mins / 60, mins % 60);
      else snprintf(dur, sizeof dur, "%dm", mins);
    }
    snprintf(meta, sizeof meta, "%s%s%s%s%s", gen, gen[0] && ano[0] ? "  \xe2\x80\xa2  " : "", ano,
             (gen[0] || ano[0]) && dur[0] ? "  \xe2\x80\xa2  " : "", dur);
    if (meta[0] || nw > 0.0f) {
      TxtLinha l = txt_linha_corta(TXT_HERO_META_GG, meta, 255, 255, 255, 178, colW - (nw > 0.0f ? nw + 40.0f : 0.0f));
      float yc = y - 20.0f, mx = x;
      if (meta[0]) { txt_desenhar_alpha(l, mx, yc - (float)l.h * 0.5f, at); mx += (float)l.w; }
      if (meta[0] && nw > 0.0f) {
        TxtLinha d = txt_linha(TXT_HERO_META_GG, "\xe2\x80\xa2", 255, 255, 255, 120);
        txt_desenhar_alpha(d, mx + 14.0f, yc - (float)d.h * 0.5f, at); mx += 14.0f + (float)d.w + 14.0f;
      }
      notaPP(i, mx, yc, at);
      y -= 40.0f + 24.0f;
    } }
  { const char *lg = extras_relacionado_logo(i);
    GLuint t = lg[0] ? tex_obter_larg(lg, colW * 0.76f) : 0;
    float ap = t ? tex_aspecto(lg) : 0.0f;
    if (t && ap > 0.0f) {
      float w = fminf(colW * 0.76f, PP_LOGO_H * ap), h = w / ap;
      y -= h;
      gfx_rect((GfxRect){ x, y, w, h }, t, GFX_TEXTO, 0, 0, 0, 0, 1, 1, 1, at);
    } else {
      TxtLinha l = txt_linha_corta(TXT_TITULO1, extras_relacionado_titulo(i), 255, 255, 255, 255, colW);
      y -= (float)l.h;
      txt_desenhar_alpha(l, x, y, at);
    }
    y -= 24.0f; }
  { char k[200];
    if (ci && ci->titulo[0]) snprintf(k, sizeof k, i18n("Porque você assistiu %s"), ci->titulo);
    else snprintf(k, sizeof k, "%s", i18n("Recomendado para você"));
    { TxtLinha l = txt_linha_corta(TXT_ILHA_CORPO, k, 255, 255, 255, 140, colW);
      txt_desenhar_alpha(l, x, y - (float)l.h, a); } }
  // Trailer preview (Android, setting On): a window mirroring the video's, on
  // the backdrop's free top-left — a full-screen trailer would sit over the
  // receded video, which shares the same Android layer.
  if (janelaReal.w > 0.0f) {
    GfxRect pr = { PP_COL_X, janelaReal.y / s, janelaReal.w / s, janelaReal.h / s };
    postplay_preview_draw(extras_relacionado_imdb(i), a > .99f && ajustes_posplay_trailer(),
                          ajustes_posplay_trailer_espera_ms(), agora, pr);
  } else postplay_preview_close();
}

// --- FILM, PHONE: the poster row (unchanged layout) --------------------------
static void desenharFilmeTelefone(Uint32 agora, float a) {
  int n = nRel(), i;
  float x = NV_ILHA_MARGEM_D;
  if(!n)postplay_preview_close();
  float sobe = (1.0f - a) * 20.0f;
  float extra = 40.0f;
  GfxRect ilha = { x, NV_TELA_H - 444.0f - extra + sobe, NV_TELA_W - 2.0f * x, 444.0f + extra - 40.0f };
  float y = ilha.y + 26.0f, cx;
  plrui_material_video(ilha, 36.0f, a);
  { float kx = ilha.x + 34.0f, yc = y + 15.0f;
    plrui_kicker("Mais como este", kx, yc - 9.0f, 243, 242, 239, a * 0.45f);
    yc += 34;
    if (foco < n) {
      TxtLinha ano = txt_linha(TXT_G18R, extras_relacionado_ano(foco), 243, 242, 239, 128);
      float nomeW = ilha.w - 68 - (ano.w ? ano.w + 16 : 0);
      TxtLinha t = txt_linha_corta(TXT_ILHA_SECAO, extras_relacionado_titulo(foco), 243, 242, 239, 255, nomeW);
      txt_desenhar_alpha(t, kx, yc - (float)t.h * 0.5f, a);
      kx += (float)t.w + 16.0f;
      if (extras_relacionado_ano(foco)[0]) txt_desenhar_alpha(ano, kx, yc - (float)ano.h * 0.5f + 2.0f, a);
    } }
  y += 30.0f + 22.0f + extra;
  cx = ilha.x + 34.0f;
#ifdef NV_TOUCH_UI
  { float vista = ilha.w - 54.0f, max = fmaxf(0, n * 212.0f - 22.0f - vista);
    toquerol_vincular(&toquePP, (GfxRect){cx, y - 20, vista, 313}, gfx_escala(), 0, max, 0, &scrollPP);
    if (!toquerol_livre(&toquePP)) {
      float esq = foco * 212.0f, dir = esq + 190;
      if (esq < scrollPP) scrollPP = esq;
      if (dir > scrollPP + vista) scrollPP = dir - vista;
      scrollPP = toquerol_clamp(scrollPP, 0, max);
    }
    if (visivel && a > .99f) ponteiro_rolagem(toquePPRolar);
    cx -= scrollPP; }
#endif
  gfx_recorte(ilha.x, ilha.y - 40.0f, ilha.w, ilha.h + 40.0f);
  for (i = 0; i < n; i++) {
    const char *po = posterprov_card(extras_relacionado_imdb(i), 0, "movie", extras_relacionado_poster(i));
    GLuint t = po[0] ? tex_obter_larg(po, 200.0f) : 0;
    int sel = (i == foco);
    float k = sel ? 1.08f : 1.0f, w = 182.0f * k, h = 273.0f * k;
    GfxRect r = { cx + (182.0f - w) * 0.5f, y + 273.0f - h, w, h };
    if (visivel && a > 0.99f && a < 1.01f) {
      float ax = cx, aw = 182.0f + 30.0f, right = fminf(ax + aw, ilha.x + ilha.w - 20);
      ax = fmaxf(ax, ilha.x + 34); aw = right - ax;
      if (aw > 0) ponteiro_alvo(ax, y - 20.0f, aw, 313.0f, ponteiroCartaz, NULL, i, 0);
    }
    if (sel) gfx_rect((GfxRect){ r.x - 20.0f, r.y + 4.0f, r.w + 40.0f, r.h + 40.0f }, 0, GFX_SOMBRA,
                      1.0f, 0, 0, 0.5f, 0, 0, 0, 0.55f * a);
    if (t) {
      gfx_tex_aspect_atual = tex_aspecto(po);
      gfx_card_para(NV_CARD_POSTERS, r, t, GFX_CARD, sel?1.0f:0.0f, 0, 0, 14.0f / r.h, 0, 0, 0, a * (sel ? 1.0f : 0.82f));
      gfx_tex_aspect_atual = 0.0f;
    } else gfx_cor(r, 14.0f / r.h, .133f, .133f, .133f, a);
    if(sel)postplay_preview_draw(extras_relacionado_imdb(i),a>.99f&&ajustes_posplay_trailer(),ajustes_posplay_trailer_espera_ms(),agora,r);
    cx += 182.0f + 30.0f;
  }
  gfx_sem_recorte();
}

// O TOPO do que o painel ocupa, para quem empilha acima dele (o cartao de
// reacao, reacao.h). As contas sao as de posplay_desenhar, sem a mola.
float posplay_topo(float baseY) {
  if (anim < 0.01f) return baseY;
  if (serie || aguardaCena) {
    int duas = aguardaCena && temProximoJogavel();
    return NV_TELA_H - 60.0f - (18.0f * 2 + PP_THUMB_H2) - (duas ? 64.0f : 0.0f) - 16.0f;
  }
  // TV film: the page covers the screen; stack above its bottom-left column.
  return telefoneui_ativo() ? NV_TELA_H - 444.0f - 40.0f - 16.0f : NV_TELA_H * 0.40f;
}

#ifdef NV_SHOT_HOOKS
// Capturas: o cartao de proximo episodio (serie) ou os relacionados (filme)
// no ar, sem esperar o fim do titulo. `fecha` = o instante da contagem.
void posplay_shot(int idxCatalogo, int ehSerie, int t, int e, Uint32 fecha) {
  fixarTitulo(idxCatalogo);
  visivel = 1; anim = 1.0f; serie = ehSerie; foco = 0; botaoFilme = 0; relAnterior = -1; relTroca = 1.0f; dispensado = 0;
  proxT = t; proxE = e; fecharEm = fecha;
  { int i, n = cat_n_episodios(idx); proxFuturo = 0;
    for (i = 0; i < n; i++) { const CatEp *x = cat_episodio(idx, i);
      if (x && x->temporada == t && x->episodio == e) {
        proxFuturo = posplay_episodio_futuro(x, (long long)time(NULL) * 1000LL); break; } } }
}
// Post-credits choice / "Are you still watching?" for captures.
void posplay_shot_estado(int cena, int aindaAssistindo) {
  shotEstado = cena || aindaAssistindo; shotCena = cena; shotAinda = aindaAssistindo;
  aguardaCena = cena; stillWatchingPrompt = aindaAssistindo;
}
#endif
