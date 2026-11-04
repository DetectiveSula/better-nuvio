#ifndef NUVIO_DEBRID_ADD_LEDGER_H
#define NUVIO_DEBRID_ADD_LEDGER_H
// Private durable receipts, scoped to the TorBox credential and torrent hash.
// 1 = newly claimed; 0 = previous attempt (*id > 0 if confirmed); -1 = no durable claim.
int debrid_tb_adicao_claim(const char *key,const char *hash,int *id);
int debrid_tb_adicao_salvar(const char *key,const char *hash,int id);
#endif
