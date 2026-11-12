// A REGRA DA TELA DE ESCOLHA DE PERFIL, sem SDL e sem rede.
//
// O que este teste prova, e por que cada coisa importa:
//
//   1. QUANDO A TELA APARECE. Zero perfis e um perfil destravado nao perguntam
//      nada; um perfil TRAVADO pergunta (senao o PIN estaria desligado); dois
//      ou mais perguntam UMA VEZ POR SESSAO.
//   2. QUE ELA VOLTA NO SEGUNDO ARRANQUE. Era o defeito: perfil.txt ligava a
//      bandeira de "ja escolheu" e a tela nunca mais aparecia — numa TV de sala
//      isso faz o app gravar o progresso no perfil de quem desligou ontem.
//   3. QUE ELA TEM O QUE DESENHAR NO PRIMEIRO QUADRO. A lista vem de um cache
//      em disco; sem ele a tela so poderia abrir depois da rede responder, ou
//      seja POR CIMA de uma home ja visivel.
//   4. QUAL PERFIL NASCE FOCADO. O ativo, nao o primeiro.
//   5. O QUE O PIN ERRADO FAZ: nada. O perfil ativo nao se mexe.
//
// Duas ETAPAS, em dois processos, porque "o segundo arranque" nao e um estado
// que se possa fabricar dentro do primeiro: a memoria do modulo tem de morrer
// junto com o processo, e so o disco atravessar.
//
//   tests/perfilsel.sh
#include "perfis.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvtmp.h"
static int servidoresTrocas, sessoesEncerradas;
static void encerrarSessao(void) { sessoesEncerradas++; }
void servidores_perfil_trocou(void) { servidoresTrocas++; } // not linked here
void plugins_perfil_mudou(void) {}     // plugins.c is not linked here

// --- pasta de dados de mentira ----------------------------------------------
static const char *pasta(void) {
  const char *p = getenv("NUVIO_TESTE_DIR");
  return p ? p : nvt_dir();
}
static void caminho(char *dst, size_t tam, const char *nome) {
  snprintf(dst, tam, "%s/%s", pasta(), nome);
}
char *dados_ler(const char *nome) {
  char c[512]; long z; char *b; FILE *f;
  caminho(c, sizeof c, nome);
  f = fopen(c, "rb");
  if (!f) return NULL;
  fseek(f, 0, SEEK_END); z = ftell(f); fseek(f, 0, SEEK_SET);
  b = malloc((size_t)z + 1);
  if (!b) { fclose(f); return NULL; }
  if (fread(b, 1, (size_t)z, f) != (size_t)z) { free(b); fclose(f); return NULL; }
  b[z] = 0; fclose(f); return b;
}
int dados_gravar(const char *nome, const char *conteudo) {
  char c[512]; FILE *f;
  caminho(c, sizeof c, nome);
  f = fopen(c, "wb");
  if (!f) return 0;
  fputs(conteudo, f); fclose(f); return 1;
}
// fileiras.c nao entra neste teste; o perfil so avisa a troca.
void fil_definir_perfil(int p) { (void)p; }
static int homeProjection;
int fil_tem_projecao(void) { return homeProjection; }
// fontepref.c idem: a fonte lembrada tambem e por perfil, e perfis.c so avisa.
void fontepref_definir_perfil(int p) { (void)p; }
void legmem_definir_perfil(int p) { (void)p; }
void audiomem_definir_perfil(int p) { (void)p; }
// arteescolha.c idem (#142): a arte escolhida a mao e por perfil.
void arteesc_definir_perfil(int p) { (void)p; }
// O ajuste "Usar os addons do perfil principal" (ajustes.c): o teste escolhe.
static int addonsDoPrincipal = 1;
int ajustes_addons_do_principal(void) { return addonsDoPrincipal; }
int dados_apagar(const char *nome) {
  char c[512]; caminho(c, sizeof c, nome); return remove(c) == 0;
}

// --- servidor de mentira ------------------------------------------------------
//
// As respostas sao as do contrato (PLANO-CONTA-SYNC secao 1.5, mapProfileRow) e
// nao inventadas: e por isso que este teste pega quando um campo novo deixa de
// ser lido. `sync_pull_profile_locks` devolve UMA LINHA POR PERFIL, com
// pin_enabled false onde nao ha PIN — que foi a armadilha que ja trancou todos
// os perfis de uma conta de uma vez.
static const char *PERFIS_JSON =
  "[{\"profile_index\":1,\"name\":\"Henrique\",\"avatar_color_hex\":\"#1E88E5\","
  "\"avatar_id\":\"avatar_lalo\",\"avatar_url\":null,"
  "\"profile_background_id\":\"bg_1\","
  "\"profile_background_url\":\"https://exemplo.invalido/fundo1.jpg\","
  "\"uses_primary_addons\":true,\"uses_primary_plugins\":false,\"is_primary\":true},"
  "{\"profile_index\":2,\"name\":\"Álvaro\",\"avatar_color_hex\":\"#E53935\","
  "\"avatar_url\":\"https://exemplo.invalido/av2.png\","
  "\"profile_background_url\":null,"
  "\"uses_primary_plugins\":true,\"is_primary\":false},"
  "{\"profile_index\":3,\"name\":\"Infantil\",\"avatar_color_hex\":\"#43A047\","
  "\"avatar_url\":null,\"profile_background_url\":null,"
  "\"uses_primary_plugins\":false,\"is_primary\":false}]";

static const char *LOCKS_JSON =
  "[{\"profile_id\":1,\"pin_enabled\":false},"
  "{\"profile_id\":2,\"pin_enabled\":false},"
  "{\"profile_id\":3,\"pin_enabled\":true}]";

static const char *UM_PERFIL_JSON =
  "[{\"profile_index\":1,\"name\":\"Sozinho\",\"avatar_color_hex\":\"#1E88E5\"}]";
static const char *UM_LOCK_LIVRE = "[{\"profile_id\":1,\"pin_enabled\":false}]";
static const char *UM_LOCK_TRAVADO = "[{\"profile_id\":1,\"pin_enabled\":true}]";

// O que o servidor de mentira devolve nesta rodada.
static const char *respPerfis = NULL, *respLocks = NULL;
// O PIN que o "servidor" aceita. Guardado aqui, no teste, e nunca no app.
static const char *pinBom = "4271";
static int pinsPedidos;
// 1 = o servidor nao responde (TV sem rede). Existe para provar que "nao deu
// para perguntar" nao se confunde com "voce errou o PIN".
static int redeCaida;

const char *sessao_usuario(void) { return "usuario-de-teste"; }
int sessao_identidade_local_copiar(char *usuario, size_t nUsuario, char *jwt, size_t nJwt) {
  snprintf(usuario, nUsuario, "%s", sessao_usuario());
  snprintf(jwt, nJwt, "test-token");
  return 1;
}
void sessao_token_recusado(const char *tok) { (void)tok; assert(0); }
char *sessao_rpc(const char *funcao, const char *corpoJson, int *status);

// O catalogo de avatares oficiais e uma RPC ANONIMA (get_avatar_catalog). Aqui
// ele devolve vazio de proposito: a regra que este teste guarda e QUANDO a tela
// aparece e em que perfil o cursor nasce, e nada disso depende de haver foto.
// Devolver NULL exercita o caminho de "sem catalogo", que e o de quem esta sem
// rede — e ai o avatar continua sendo a inicial no circulo colorido.
const char *nuvem_url(void) { return "https://exemplo.invalido"; }
char *nuvem_rpc_com(const char *funcao, const char *corpoJson,
                    const char *bearer, int *status) {
  if (bearer) {
    assert(!strcmp(bearer, "test-token"));
    return sessao_rpc(funcao, corpoJson, status);
  }
  if (status) *status = 0;
  return NULL;
}

char *sessao_rpc(const char *funcao, const char *corpoJson, int *status) {
  const char *r = NULL;
  if (status) *status = 200;
  if (!strcmp(funcao, "get_sync_owner")) r = "\"441bf572-0000-4000-8000-000000000000\"";
  else if (!strcmp(funcao, "sync_pull_profiles")) r = respPerfis;
  else if (!strcmp(funcao, "sync_pull_profile_locks")) r = respLocks;
  else if (!strcmp(funcao, "verify_profile_pin")) {
    pinsPedidos++;
    // Sem rede o transporte devolve NULL e status 0, que e o que sessao_rpc faz
    // quando a requisicao nem sai.
    if (redeCaida) { if (status) *status = 0; return NULL; }
    // O PIN VIAJA NO CORPO, e o app tem de monta-lo com o escritor de JSON.
    // Aqui basta procurar o valor certo: se o corpo estivesse malformado (uma
    // aspa nao escapada, por exemplo), a busca falharia e o teste acusaria.
    r = strstr(corpoJson ? corpoJson : "", pinBom) ? "true" : "false";
  }
  return r ? strdup(r) : NULL;
}

// -----------------------------------------------------------------------------
static void etapaUm(void) {
  const ContaPerfil *p;

  // A) Nada puxado ainda: sem perfis nao ha pergunta. E o caso de quem NAO TEM
  //    CONTA — a lista nunca chega, e a tela nunca deve aparecer.
  assert(perfis_n() == 0);
  assert(perfis_sem_escolha() == 1);
  assert(perfis_precisa_escolher() == 0);
  assert(perfis_pode_dispensar() == 0);
  assert(perfis_acao(0) == PERFIL_ACAO_NADA);

  // B) Um perfil so, destravado: continua sem pergunta. Uma tela com uma
  //    resposta possivel e um clique cobrado a toa a cada abertura.
  respPerfis = UM_PERFIL_JSON; respLocks = UM_LOCK_LIVRE;
  assert(perfis_puxar() == 1);
  assert(perfis_sem_escolha() == 1);
  assert(perfis_precisa_escolher() == 0);

  // C) O MESMO perfil, agora com PIN: a pergunta volta. Pular aqui seria o
  //    mesmo que desligar a trava.
  respLocks = UM_LOCK_TRAVADO;
  assert(perfis_puxar() == 1);
  assert(perfis_item(0)->temPin == 1);
  assert(perfis_sem_escolha() == 0);
  assert(perfis_precisa_escolher() == 1);
  assert(perfis_acao(0) == PERFIL_ACAO_PIN);
  // E o Voltar nao pode servir de chave: sem escolha gravada, nao dispensa.
  assert(perfis_pode_dispensar() == 0);

  // D) A conta de verdade: tres perfis, o terceiro travado.
  respPerfis = PERFIS_JSON; respLocks = LOCKS_JSON;
  assert(perfis_puxar() == 3);
  p = perfis_item(0);
  assert(!strcmp(p->nome, "Henrique") && !strcmp(p->corHex, "#1E88E5"));
  assert(p->primario == 1 && p->temPin == 0);
  // profile_background_url e o campo novo: e dele que sai a arte por tras da
  // tela. Sem esta linha o campo pode parar de ser lido sem ninguem notar.
  assert(!strcmp(p->fundoUrl, "https://exemplo.invalido/fundo1.jpg"));
  p = perfis_item(1);
  assert(!strcmp(p->nome, "Álvaro"));
  assert(!strcmp(p->avatarUrl, "https://exemplo.invalido/av2.png"));
  assert(p->fundoUrl[0] == 0 && p->usaAddonsDoPrimario == 1);
  p = perfis_item(2);
  assert(!strcmp(p->nome, "Infantil") && p->temPin == 1);

  assert(perfis_sem_escolha() == 0);
  assert(perfis_precisa_escolher() == 1);
  assert(perfis_acao(0) == PERFIL_ACAO_ENTRAR);
  assert(perfis_acao(2) == PERFIL_ACAO_PIN);
  assert(perfis_acao(3) == PERFIL_ACAO_NADA);

  // E) Sem nada gravado, o cursor nasce no primeiro.
  assert(perfis_ativo() == 1 && perfis_indice_sugerido() == 0);

  // F) PIN ERRADO NAO ENTRA. Nem muda o perfil ativo: e a diferenca entre uma
  //    trava e um aviso.
  assert(perfis_verificar_pin(3, "0000") == 0);
  assert(perfis_ativo() == 1);
  assert(perfis_verificar_pin(3, pinBom) == 1);
  assert(pinsPedidos == 2);

  //    E O DEFEITO QUE ISTO GUARDA: sem rede, o resultado tem de ser -1 e NAO
  //    0. Com os dois juntos, a TV desconectada dizia "PIN incorreto" a quem
  //    digitou o PIN certo — a tela ate tinha o aviso "Sem conexao", so que
  //    nenhum caminho chegava nele.
  redeCaida = 1;
  assert(perfis_verificar_pin(3, pinBom) == -1);
  assert(pinsPedidos == 3);
  redeCaida = 0;
  assert(perfis_verificar_pin(3, pinBom) == 1);
  // O PIN so vale depois que a tela grava o perfil — e isso e explicito.
  assert(perfis_ativo() == 1);

  // G) Escolher encerra a pergunta DESTA sessao, e so dela.
  perfis_ao_abrir_sessao(encerrarSessao);
  perfis_definir_ativo(2);
  assert(perfis_ativo() == 2);
  assert(perfis_precisa_escolher() == 0);
  assert(perfis_sem_escolha() == 0);      // a lista nao mudou
  assert(perfis_pode_dispensar() == 1);
  assert(perfis_indice_sugerido() == 1);
  // Reopening the same profile intentionally starts a fresh session, including
  // player teardown and personal-server reload; old publications are rejected.
  { uint64_t anterior = perfis_sessao_geracao();
    int encerradas = sessoesEncerradas, trocas = servidoresTrocas;
    perfis_definir_ativo(2);
    assert(perfis_ativo() == 2 && !perfis_sessao_atual(anterior));
    assert(!perfis_sessao_travar(anterior));
    assert(sessoesEncerradas == encerradas + 1 && servidoresTrocas == trocas + 1); }

  // G2) ADDONS DO PRINCIPAL. O perfil 2 tem a marca da conta: le os do 1 com o
  //     ajuste local ligado ou desligado. O perfil 3 NAO tem a marca: so o
  //     ajuste local (padrao ligado) o manda para o 1; desligado, e o dele.
  addonsDoPrincipal = 0;
  assert(perfis_ativo_addons() == 1);
  perfis_definir_ativo(3);
  assert(perfis_ativo_addons() == 3);
  addonsDoPrincipal = 1;
  assert(perfis_ativo_addons() == 1);
  // A valid Stage G projection owns its catalogue scope. Keep the legacy
  // fallback above, but do not let it override a synced profile's own addons.
  homeProjection = 1;
  assert(perfis_home_config() == 3 && perfis_ativo_addons() == 3);
  perfis_definir_ativo(2);
  assert(perfis_home_config() == 1 && perfis_ativo_addons() == 1);
  homeProjection = 0;
  perfis_definir_ativo(1);
  assert(perfis_ativo_addons() == 1);
  perfis_definir_ativo(2);

  // H) O cache nao pode guardar segredo. Ele existe para desenhar a tela, e o
  //    PIN nunca chega a este modulo em texto — muito menos ao disco.
  { char *c = dados_ler("perfis.txt");
    assert(c && strstr(c, "Henrique") && strstr(c, "Infantil"));
    assert(!strstr(c, pinBom));
    free(c); }

  { ContaPerfil before=*perfis_item(1),other=*perfis_item(0);int active=perfis_ativo();
    uint64_t generation=perfis_sessao_geracao();assert(perfis_sessao_travar(generation));
    assert(perfis_aplicar_edicao(2,"Renamed","https://example.invalid/photo.jpg"));
    assert(!strcmp(perfis_item(1)->nome,"Renamed"));assert(perfis_ativo()==active);
    assert(perfis_item(1)->usaAddonsDoPrimario==before.usaAddonsDoPrimario);
    assert(perfis_item(1)->temPin==before.temPin && !strcmp(perfis_item(1)->fundoUrl,before.fundoUrl));
    assert(!memcmp(&other,perfis_item(0),sizeof other));
    assert(!perfis_aplicar_edicao(99,"Missing",""));assert(!perfis_aplicar_edicao(2,"",""));
    assert(perfis_aplicar_edicao(2,before.nome,before.avatarUrl));
    // Colour/background: NULL keeps the field, "" is the standard background.
    assert(perfis_aplicar_aparencia(2,"#E53935",NULL));
    assert(!strcmp(perfis_item(1)->corHex,"#E53935") && !strcmp(perfis_item(1)->fundoUrl,before.fundoUrl));
    assert(perfis_aplicar_aparencia(2,NULL,"https://example.invalid/wall.jpg"));
    assert(!strcmp(perfis_item(1)->corHex,"#E53935") && !strcmp(perfis_item(1)->fundoUrl,"https://example.invalid/wall.jpg"));
    assert(!perfis_aplicar_aparencia(2,"red",NULL) && !perfis_aplicar_aparencia(2,"#12345G",NULL));
    assert(!perfis_aplicar_aparencia(99,"#E53935",NULL));
    { char *c=dados_ler("perfis.txt");assert(c && strstr(c,"#E53935") && strstr(c,"wall.jpg"));free(c); }
    assert(perfis_aplicar_aparencia(2,before.corHex,before.fundoUrl));
    // PIN state follows the editor; the PIN itself never reaches this module.
    assert(perfis_aplicar_pin(2,!before.temPin) && perfis_item(1)->temPin==!before.temPin);
    assert(perfis_aplicar_pin(2,before.temPin) && perfis_item(1)->temPin==before.temPin);
    { const ContaPerfil *q=perfis_item(1);
      assert(q->indice==before.indice && q->temPin==before.temPin && q->primario==before.primario &&
             q->usaAddonsDoPrimario==before.usaAddonsDoPrimario && !strcmp(q->nome,before.nome) &&
             !strcmp(q->corHex,before.corHex) && !strcmp(q->avatarUrl,before.avatarUrl) && !strcmp(q->fundoUrl,before.fundoUrl)); }
    assert(!memcmp(&other,perfis_item(0),sizeof other));
    // Never delete profile 1, the primary or the active profile.
    { int count=perfis_n();
      assert(!perfis_aplicar_remocao(1) && !perfis_aplicar_remocao(active) && !perfis_aplicar_remocao(99));
      assert(perfis_n()==count); }
    perfis_sessao_destravar();
  }
  puts("etapa 1 (a conta chega pela rede): OK");
}

static void etapaDois(void) {
  const ContaPerfil *p;

  // Processo NOVO: nada em memoria, servidor mudo (avioes no ar, TV religada).
  respPerfis = NULL; respLocks = NULL;
  assert(perfis_n() == 0);

  perfis_carregar_ativo();

  // 1. A LISTA VEIO DO DISCO. E isto que deixa a tela abrir no primeiro quadro
  //    em vez de aparecer por cima de uma home ja montada.
  assert(perfis_n() == 3);
  p = perfis_item(0);
  assert(!strcmp(p->nome, "Henrique"));
  assert(!strcmp(p->corHex, "#1E88E5"));
  assert(!strcmp(p->fundoUrl, "https://exemplo.invalido/fundo1.jpg"));
  assert(!strcmp(perfis_item(1)->nome, "Álvaro"));
  assert(perfis_item(1)->usaAddonsDoPrimario == 1);
  // A TRAVA sobrevive ao cache. Se ela nao sobrevivesse, o perfil travado
  // abriria sem PIN sempre que o app arrancasse sem rede — que e justamente
  // quando ninguem consegue verificar nada.
  assert(perfis_item(2)->temPin == 1);

  // 2. A PERGUNTA VOLTA. Este e o defeito consertado: antes, perfil.txt ligava
  //    a bandeira de "ja escolheu" e a tela nunca mais aparecia.
  assert(perfis_ativo() == 2);
  assert(perfis_precisa_escolher() == 1);

  // 3. E ela abre com o cursor em quem estava, nao no primeiro da fila.
  assert(perfis_indice_sugerido() == 1);

  // 4. O Voltar dispensa: ha resposta de ontem e ela nao esta atras de um PIN.
  assert(perfis_pode_dispensar() == 1);
  perfis_manter_ativo();
  assert(perfis_precisa_escolher() == 0);
  assert(perfis_ativo() == 2);            // manter nao troca ninguem

  // 5. Sair da conta leva a lista embora. Deixar o cache no aparelho faria a
  //    proxima conta abrir mostrando os nomes da familia anterior.
  perfis_esquecer();
  assert(perfis_n() == 0 && perfis_ativo() == 1);
  assert(dados_ler("perfis.txt") == NULL);
  assert(dados_ler("perfil.txt") == NULL);

  puts("etapa 2 (o segundo arranque, sem rede): OK");
}

// A profile deleted in the editor leaves the list and the disk cache at once;
// the others keep their order and fields. Separate process: it rewrites the cache.
static void etapaRemocao(void) {
  respPerfis = PERFIS_JSON; respLocks = LOCKS_JSON;
  assert(perfis_puxar() == 3);
  perfis_definir_ativo(1);
  ContaPerfil um = *perfis_item(0), tres = *perfis_item(2);
  uint64_t g = perfis_sessao_geracao(); assert(perfis_sessao_travar(g));
  assert(!perfis_aplicar_remocao(1));
  assert(perfis_aplicar_remocao(2));
  assert(perfis_n() == 2 && !memcmp(&um, perfis_item(0), sizeof um) && !memcmp(&tres, perfis_item(1), sizeof tres));
  assert(!perfis_aplicar_remocao(2));
  perfis_sessao_destravar();
  { char *c = dados_ler("perfis.txt"); assert(c && !strstr(c, "\u00c1lvaro") && !strstr(c, "Álvaro") && strstr(c, "Infantil")); free(c); }
  puts("remocao (perfil apagado no editor): OK");
}

int main(int argc, char **argv) {
  if (argc > 1 && !strcmp(argv[1], "dois")) { etapaDois(); return 0; }
  if (argc > 1 && !strcmp(argv[1], "remocao")) { etapaRemocao(); return 0; }
  etapaUm();
  return 0;
}
