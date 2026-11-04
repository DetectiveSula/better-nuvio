#ifndef NUVIO_RESOLVER_POLICY_H
#define NUVIO_RESOLVER_POLICY_H
// Policy only: all actual provider resolution stays in debrid.c.
int resolver_politica_mascara(void);
int resolver_politica_provedor(int preferred); // stored 0=automatic, 1=TorBox, 2=Premiumize
int resolver_politica_opcao(int value);
int resolver_politica_limite(int stored);
int resolver_politica_resolver(int requested, const char *hash, int file,
                              char *url, unsigned size, int *outside);
#endif
