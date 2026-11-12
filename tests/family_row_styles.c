/* Exercise the actual Family renderer with deterministic metadata and draw
 * recording: no account, network, GL context or persisted user preferences. */
#include "../src/familia_ui.c"
#include <assert.h>
#include <math.h>
static int style, texts, badges, rectangles;
static float bottom, width;
float gfx_tex_aspect_atual;
int ajustes_amigos_estilo(void) { return style; }
int familia_cartao_do_perfil(int perfil, FamCartao *out) {
  memset(out,0,sizeof *out); out->perfil=perfil;strcpy(out->nome,"Test profile");
  out->estado=FAM_EM_CURSO;out->pct=42;return 1;
}
void familia_rotulos(const FamCartao *c,long long now,char *p,size_t np,char *l,size_t nl) {
  snprintf(p,np,"In Progress");snprintf(l,nl,"S2 E5 · 22m left");
}
static void bounds(float x,float y,float w,float h) {
  assert(x>=-0.01f && y>=-0.01f && x+w<=width+0.01f && y+h<=bottom+0.01f);
}
void gfx_veu_base(GfxRect r,float rad,float f,float a) { bounds(r.x,r.y,r.w,r.h); }
void gfx_cor(GfxRect r,float rad,float cr,float cg,float cb,float ca) { bounds(r.x,r.y,r.w,r.h);rectangles++; }
void gfx_rect(GfxRect r,GLuint tex,GfxModo m,float f,float px,float py,float rad,float cr,float cg,float cb,float a) { bounds(r.x,r.y,r.w,r.h); }
void gfx_icone(GfxRect r,const char *n,float cr,float cg,float cb,float a) { bounds(r.x,r.y,r.w,r.h); }
GLuint tex_obter_larg(const char *u,float w){return 0;}
float tex_aspecto(const char *u){return 1;}
TxtLinha txt_linha_corta(TxtEstilo e,const char *s,int r,int g,int b,int a,float w) {
  return (TxtLinha){.w=(int)fminf(w,strlen(s)*8),.h=22};
}
TxtLinha txt_linha(TxtEstilo e,const char *s,int r,int g,int b,int a) { return txt_linha_corta(e,s,r,g,b,a,1000); }
void txt_desenhar(TxtLinha l,float x,float y){bounds(x,y,l.w,l.h);texts++;}
void txt_desenhar_alpha(TxtLinha l,float x,float y,float a){txt_desenhar(l,x,y);}
float badge_largura(const char *s){return 100;}
float badge_desenhar(float x,float y,const char *s,BadgeEstilo e,float a){bounds(x,y,100,22);badges++;return 100;}
int main(void) {
  CatItem item={0};strcpy(item.socialSlug,"familia:2");strcpy(item.titulo,"Fixture episode");
  for(style=0;style<6;style++) {
    width=familia_estilo_largura(style);bottom=familia_estilo_altura(style);
    for(int i=0;i<style;i++)assert(width!=familia_estilo_largura(i)||bottom!=familia_estilo_altura(i));
    texts=badges=rectangles=0;
    familia_desenhar_cartao(&item,(GfxRect){0,0,width,bottom},8,1);
    assert(texts==(style==5?3:4));
    assert(badges==(style==3||style==5?0:1));
    assert(rectangles==(style==5?1:3)); // profile chip plus progress track/fill
  }
  puts("PASS: all six Family row styles change geometry, preserve readable overlays and remain inside card bounds");
}
