/* Actual Native bootstrap -> Home mutation -> J4/J3 worker -> SDL apply. */
#include "j5-native-fixture.inc"
int main(int argc,char **argv){
 assert(argc==2);setenv("NUVIO_DADOS",argv[1],1);dados_iniciar(NULL);player_dir(argv[1]);legfontes_dir(argv[1]);ajustes_dir(argv[1]);fil_definir_perfil(1);login();sync_reaplicar_ajustes();jbNextApply=0;ajustes_better_passo();
 const char *ids[]={"j5.fixture"},*bases[]={"https://fixture.invalid/private-config"};fil_podar_catalogos(ids,bases,1,1);const char *key="j5.fixture_movie_top";fil_registrar(key,"Fixture","Fixture","movie",1);
 jbNextApply=0;ajustes_better_passo();assert(jbCurrent&&!bna_writes_enabled(jbCurrent->account));
 assert(fil_definir_tipo(key,FIL_TIPO_FICHA));char id[33],field[40];assert(fil_home_identity(key,id));snprintf(field,sizeof field,"home_%s_s",id);BnValue v;assert(bna_get(jbCurrent->account,field,&v)&&v.number==FIL_TIPO_FICHA);
 ajustes_better_ciclo(nv_perfil_sessao_ler());assert(!betterPushes);assert(ajustes_better_writes(1));ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==1);
 /* Simulated peer edits the very same typed document, then actual worker pulls. */
 BnDef *d=calloc(JB_N+1,sizeof *d);BnEngine *e=calloc(1,sizeof *e);BnDoc *doc=calloc(1,sizeof *doc);assert(d&&e&&doc&&jbDefs(valor,1,d));home_definition(&d[JB_N]);assert(bn_init(e,sessao_usuario(),1,"peer-device",d,JB_N+1));
 char *response=malloc(strlen(betterCloud)+40);assert(response);sprintf(response,"[{\"settings_json\":%s}]",betterCloud);assert(bnc_response(e,response,doc)&&bn_pull(e,doc));free(response);v.number=FIL_TIPO_PALCO;assert(bn_edit(e,field,v));assert(bn_pull(e,doc)&&bn_prepare(e,doc));char *json=NULL;assert(bn_json(e,doc,&json));free(betterCloud);betterCloud=json;serverSave();
 ajustes_better_ciclo(nv_perfil_sessao_ler());jbNextApply=0;ajustes_better_passo();assert(fil_tipo(key)==FIL_TIPO_PALCO);int before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before);
 /* An inactive profile editor retains Home fields in the same checkpoint. */
 selectProfile(2);assert(ajustes_edicao_iniciar(1));valor[AJ_HERO_ANO]=!valor[AJ_HERO_ANO];assert(gravar());ajustes_edicao_encerrar();selectProfile(1);assert(bna_get(jbCurrent->account,field,&v)&&v.number==FIL_TIPO_PALCO);

 free(d);free(e);free(doc);ajustes_better_esquecer();puts("PASS: real J4 bootstrap/save callback, J3 default-Off/explicit upload, peer restore through Native SDL projection and no echo");return 0;
}
