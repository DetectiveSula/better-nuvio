// The native half of the profile editor: what it sends the Android panel and
// what it applies from the panel's events. No Android, no network: the JNI
// calls are replaced by a queue, the account by the perfilsel fixtures.
#define main perfilsel_fixture_main
#include "perfilsel.c"
#undef main
#include "profile_editor.h"
#include "js.h"

static char opened[16384];
static const char *queue[8];
static int queued, closes;
int android_profile_editor_open(const char *json) { snprintf(opened, sizeof opened, "%s", json); return 1; }
void android_profile_editor_close(void) { closes++; }
int android_profile_editor_event(char *dst, size_t n) {
  if (!queued) return 0;
  snprintf(dst, n, "%s", queue[0]); memmove(queue, queue + 1, sizeof queue - sizeof queue[0]); queued--; return 1;
}
static void push(const char *e) { queue[queued++] = e; }
const char *nuvem_anon(void) { return "anon-key"; }
void ajustes_acento(float *r, float *g, float *b) { *r = 0.18f; *g = 0.83f; *b = 0.75f; }
const char *dados_cliente_id(void) { return "device-1"; }

static unsigned request(void) { return (unsigned)js_num(opened, NULL, "request", 0); }
static void send(const char *fmt) { static char b[8][512]; static int k; snprintf(b[k], sizeof b[k], fmt, request()); push(b[k]); k = (k + 1) % 8; }

int main(void) {
  respPerfis = PERFIS_JSON; respLocks = LOCKS_JSON;
  assert(perfis_puxar() == 3);
  perfis_definir_ativo(1);
  char v[64];

  // Opening hands over the profile, its role and look; a locked profile also gets the PIN just typed.
  assert(profile_editor_open(2, "4271") && profile_editor_active());
  assert(js_num(opened, NULL, "profile", 0) == 3 && js_num(opened, NULL, "active", 0) == 1);
  assert(js_texto(opened, NULL, "pin", v, sizeof v) && !strcmp(v, "4271"));
  assert(js_texto(opened, NULL, "color", v, sizeof v) && !strcmp(v, "#43A047"));
  assert(js_texto(opened, NULL, "accent", v, sizeof v) && !strcmp(v, "#2ED4BF"));
  assert(js_texto(opened, NULL, "client", v, sizeof v) && !strcmp(v, "device-1"));
  assert(strstr(opened, "\"hasPin\":true") && strstr(opened, "\"primary\":false"));

  // A PIN change keeps the panel open and updates the lock state.
  send("{\"request\":%u,\"final\":0,\"pin\":0}");
  profile_editor_update();
  assert(profile_editor_active() && perfis_item(2)->temPin == 0);
  // A save applies name, photo, colour and background together, then closes.
  send("{\"request\":%u,\"saved\":1,\"name\":\"Kids\",\"photo\":\"\",\"color\":\"#8E24AA\",\"background\":\"https://exemplo.invalido/w.jpg\"}");
  send("{\"request\":%u,\"saved\":0}");   // the close that always follows is ignored
  profile_editor_update();
  assert(!profile_editor_active());
  assert(!strcmp(perfis_item(2)->nome, "Kids") && !strcmp(perfis_item(2)->corHex, "#8E24AA"));
  assert(!strcmp(perfis_item(2)->fundoUrl, "https://exemplo.invalido/w.jpg") && perfis_item(2)->temPin == 0);

  // "" is a real value: no photo, standard background.
  assert(profile_editor_open(2, NULL));
  send("{\"request\":%u,\"saved\":1,\"name\":\"Kids\",\"photo\":\"\",\"background\":\"\"}");
  profile_editor_update();
  assert(perfis_item(2)->fundoUrl[0] == 0 && perfis_item(2)->avatarUrl[0] == 0 && !strcmp(perfis_item(2)->corHex, "#8E24AA"));

  // An unlocked profile never carries a PIN, even if one is passed.
  assert(profile_editor_open(1, "9999") && !strstr(opened, "\"pin\":"));
  // A name/photo-only save leaves colour and background alone.
  send("{\"request\":%u,\"saved\":1,\"name\":\"Alvaro\",\"photo\":\"https://exemplo.invalido/a.png\"}");
  profile_editor_update();
  assert(!strcmp(perfis_item(1)->nome, "Alvaro") && !strcmp(perfis_item(1)->corHex, "#E53935"));

  // A stale event from an earlier panel changes nothing.
  assert(profile_editor_open(1, NULL));
  push("{\"request\":1,\"deleted\":1}");
  profile_editor_update();
  assert(perfis_n() == 3 && profile_editor_active());
  // Delete removes the profile; the active one is refused natively too.
  send("{\"request\":%u,\"deleted\":1}");
  profile_editor_update();
  assert(perfis_n() == 2 && !profile_editor_active() && !strcmp(perfis_item(1)->nome, "Kids"));
  assert(profile_editor_open(0, NULL));
  send("{\"request\":%u,\"deleted\":1}");
  profile_editor_update();
  assert(perfis_n() == 2);

  // A session change closes the panel and drops its late events.
  assert(profile_editor_open(1, NULL));
  unsigned r = request();
  perfis_invalidar_sessao();
  profile_editor_update();
  assert(!profile_editor_active() && closes >= 1);
  char late[128]; snprintf(late, sizeof late, "{\"request\":%u,\"deleted\":1}", r); push(late);
  profile_editor_update();
  assert(perfis_n() == 2);
  puts("PASS: profile editor events apply colour/background/PIN/delete, ignore stale and closed panels");
  return 0;
}
