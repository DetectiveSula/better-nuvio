#include "debrid_add_ledger.h"
#include "admin.h"
#include "dados.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
static int caminho(const char *key,const char *hash,char *folder,size_t nf,char *name,size_t nn,char *path,size_t np) {
  const char *dir=dados_dir();if(!dir||!*dir||!key||!*key||!hash||strlen(hash)!=40)return 0;
  char seed[300],digest[65];int n=snprintf(seed,sizeof seed,"%s:%s",key,hash);
  if(n<0||(size_t)n>=sizeof seed)return 0;
  admin_sha256_hex((const unsigned char*)seed,(unsigned long)n,digest);memset(seed,0,sizeof seed);
  if(snprintf(folder,nf,"%s/torbox-cached-additions",dir)>=(int)nf)return 0;
  if(snprintf(name,nn,"torbox-cached-additions/%s.state",digest)>=(int)nn)return 0;
  return snprintf(path,np,"%s/%s",dir,name)<(int)np;
}
int debrid_tb_adicao_claim(const char *key,const char *hash,int *id) {
  char folder[1200],name[160],path[1400];if(id)*id=0;
  if(!caminho(key,hash,folder,sizeof folder,name,sizeof name,path,sizeof path))return -1;
  if(mkdir(folder,0700)&&errno!=EEXIST)return -1;
  int flags=O_WRONLY|O_CREAT|O_EXCL;
#ifdef O_NOFOLLOW
  flags|=O_NOFOLLOW;
#endif
  int fd=open(path,flags,0600);
  if(fd<0) {
    if(errno!=EEXIST)return -1;
    FILE *f=fopen(path,"r");if(f){int value=0;char line[64],extra;
      if(fgets(line,sizeof line,f) && sscanf(line,"item %d %c",&value,&extra)==1 && fgetc(f)==EOF && value>0 && id)*id=value;
      fclose(f);
    }
    return 0; // empty, corrupt or pending receipts must never cause another POST
  }
  const char pending[]="pending\n";
  int ok=write(fd,pending,sizeof pending-1)==sizeof pending-1 && fsync(fd)==0;
  if(close(fd))ok=0;
  int directory=open(folder,O_RDONLY);if(directory<0)ok=0;
  else {if(fsync(directory))ok=0;close(directory);}
  // Leave a receipt even on persistence failure. No account request has been sent.
  return ok?1:-1;
}
int debrid_tb_adicao_salvar(const char *key,const char *hash,int id) {
  char folder[1200],name[160],path[1400],body[40];
  if(id<=0||!caminho(key,hash,folder,sizeof folder,name,sizeof name,path,sizeof path))return 0;
  snprintf(body,sizeof body,"item %d\n",id);return dados_gravar(name,body);
}
