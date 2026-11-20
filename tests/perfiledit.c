// Profile editor rules (src/perfiledit.c, NV_PERFILEDIT_SO_REGRAS): names,
// colours, PINs, catalogue parsing and the request bodies sent to the account.
//   bash tests/perfiledit.sh
#include "perfiledit.h"
#include "js.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void nomes(void) {
  char o[80];
  assert(perfiledit_nome_valido("Ana"));
  assert(perfiledit_nome_valido("  Ana  "));
  assert(!perfiledit_nome_valido(""));
  assert(!perfiledit_nome_valido("   "));
  assert(!perfiledit_nome_valido(NULL));
  assert(!perfiledit_nome_valido("a\tb"));
  assert(perfiledit_nome_valido("12345678901234567890"));     // 20
  assert(!perfiledit_nome_valido("123456789012345678901"));   // 21
  // 20 code points of 3 bytes = 60 bytes: fine; 21 is not.
  assert(perfiledit_nome_valido("ああああああああああああああああああああ"));
  assert(!perfiledit_nome_valido("あああああああああああああああああああああ"));
  // 16 emoji of 4 bytes = 64 bytes: over the byte limit with only 16 chars.
  assert(!perfiledit_nome_valido("😀😀😀😀😀😀😀😀😀😀😀😀😀😀😀😀"));
  perfiledit_nome_aparar("  Élio  ", o, sizeof o); assert(!strcmp(o, "Élio"));
  perfiledit_nome_aparar("123456789012345678 abc", o, sizeof o); assert(!strcmp(o, "123456789012345678 a"));
  perfiledit_nome_aparar("1234567890123456789 abc", o, sizeof o); assert(!strcmp(o, "1234567890123456789"));
  perfiledit_nome_aparar("a\nb", o, sizeof o); assert(!strcmp(o, "ab"));
  perfiledit_nome_aparar("😀😀😀😀😀😀😀😀😀😀😀😀😀😀😀😀", o, sizeof o);
  assert(strlen(o) == 60 && perfiledit_nome_valido(o));       // clamped, never mid-character
}
static void regras(void) {
  char c[8];
  assert(perfiledit_cor_normal("#e53935", c) && !strcmp(c, "#E53935"));
  assert(perfiledit_cor_normal("1E88E5", c) && !strcmp(c, "#1E88E5"));
  assert(perfiledit_cor_normal("#FF8E24AA", c) && !strcmp(c, "#8E24AA"));
  assert(!perfiledit_cor_normal("#12345", c) && !perfiledit_cor_normal("#GGGGGG", c) && !perfiledit_cor_normal(NULL, c));
  assert(perfiledit_pin_valido("0123") && !perfiledit_pin_valido("123") && !perfiledit_pin_valido("12a4") &&
         !perfiledit_pin_valido("12345") && !perfiledit_pin_valido(""));
  assert(perfiledit_url_valida("") && perfiledit_url_valida("https://x.y/a.png"));
  assert(!perfiledit_url_valida("javascript:x") && !perfiledit_url_valida("https://x y") && !perfiledit_url_valida(NULL));
  for (int i = 0; i < PE_CORES; i++) assert(perfiledit_cor_normal(PE_CORES_HEX[i], c) && !strcmp(c, PE_CORES_HEX[i]));
}
static void catalogos(void) {
  PEAvatar a[8];
  PEFundo f[8];
  const char *av = "[{\"id\":\"avatar_red_fox\",\"storage_path\":\"/animals/fox.png\",\"display_name\":null,\"bg_color\":\"ff5722\"},"
                   "{\"id\":\"x\",\"storage_path\":\"../etc/passwd\"},"
                   "{\"id\":\"y\",\"storage_path\":\"a.png?x=1\"},"
                   "{\"id\":\"cat\",\"storage_path\":\"cat.png\",\"display_name\":\"Cat\"}]";
  int n = perfiledit_ler_avatares(av, "https://srv.example/", a, 8);
  assert(n == 2);
  assert(!strcmp(a[0].id, "avatar_red_fox") && !strcmp(a[0].nome, "red fox") && !strcmp(a[0].cor, "#FF5722"));
  assert(!strcmp(a[0].url, "https://srv.example/storage/v1/object/public/avatars/animals/fox.png"));
  assert(!strcmp(a[1].nome, "Cat") && a[1].cor[0] == 0);
  assert(perfiledit_ler_avatares("{}", "https://s", a, 8) == 0 && perfiledit_ler_avatares(NULL, "https://s", a, 8) == 0);
  assert(perfiledit_ler_avatares(av, "https://s", a, 1) == 1);
  n = perfiledit_ler_fundos("[{\"id\":\"aurora\",\"storage_path\":\"bg/aurora.jpg\",\"display_name\":\"Aurora\"},"
                            "{\"id\":\"bad\",\"storage_path\":\"https://evil/x.jpg\"}]", f, 8);
  assert(n == 1 && !strcmp(f[0].id, "aurora") && !strcmp(f[0].nome, "Aurora"));
}
static void corpos(void) {
  char *b = perfiledit_corpo_patch(2, "  Renamed ", 0, NULL, NULL), v[200];
  assert(b && !strcmp(b, "{\"p_profile_id\":2,\"p_name\":\"Renamed\"}"));
  free(b);
  b = perfiledit_corpo_patch(2, "Ana", 1, "avatar_cat", NULL);
  assert(b && strstr(b, "\"p_avatar_id_provided\":true") && strstr(b, "\"p_avatar_id\":\"avatar_cat\"") &&
         strstr(b, "\"p_avatar_url\":null"));
  free(b);
  b = perfiledit_corpo_patch(2, "Ana", 2, NULL, NULL);
  assert(b && strstr(b, "\"p_avatar_id\":null") && strstr(b, "\"p_avatar_url\":null"));
  free(b);
  assert(!perfiledit_corpo_patch(2, "", 0, NULL, NULL) && !perfiledit_corpo_patch(9, "Ana", 0, NULL, NULL) &&
         !perfiledit_corpo_patch(2, "Ana", 1, NULL, NULL));
  // An image link: avatar_url with no avatar_id. Only http(s) links pass.
  b = perfiledit_corpo_patch(2, "Ana", 3, NULL, "https://img.example/me.png");
  assert(b && strstr(b, "\"p_avatar_id\":null") && strstr(b, "\"p_avatar_url\":\"https://img.example/me.png\""));
  free(b);
  assert(!perfiledit_corpo_patch(2, "Ana", 3, NULL, "javascript:alert(1)") &&
         !perfiledit_corpo_patch(2, "Ana", 3, NULL, "") && !perfiledit_corpo_patch(2, "Ana", 3, NULL, NULL) &&
         !perfiledit_corpo_patch(2, "Ana", 4, NULL, NULL));

  { const char *pull =
      "[{\"profile_index\":1,\"name\":\"Main\",\"avatar_color_hex\":\"#1E88E5\",\"uses_primary_addons\":false,"
      "\"uses_primary_plugins\":false,\"avatar_id\":\"a1\",\"avatar_url\":\"https://x/a1.png\","
      "\"profile_background_id\":null,\"profile_background_url\":null,\"extra\":{\"k\":1}},"
      "{\"profile_index\":2,\"name\":\"Kid\",\"avatar_color_hex\":\"#43A047\",\"uses_primary_addons\":true,"
      "\"uses_primary_plugins\":true,\"avatar_id\":null,\"avatar_url\":null,"
      "\"profile_background_id\":\"aurora\",\"profile_background_url\":\"https://x/bg.jpg\"}]";
    const char *e, *f;
    b = perfiledit_corpo_push(pull, 2, "Kiddo", 0, NULL, NULL, "#e53935", 1, NULL, "client-1");
    assert(b);
    assert(strstr(b, "\"p_client_max_profiles\":6") && strstr(b, "\"p_origin_client_id\":\"client-1\""));
    e = js_array(b, NULL, "p_profiles");
    assert(e);
    // Profile 1 is copied back unchanged.
    f = js_fim(e);
    assert((int)js_num(e, f, "profile_index", 0) == 1);
    v[0] = 0; js_texto(e, f, "name", v, sizeof v); assert(!strcmp(v, "Main"));
    v[0] = 0; js_texto(e, f, "avatar_url", v, sizeof v); assert(!strcmp(v, "https://x/a1.png"));
    v[0] = 0; js_texto(e, f, "avatar_color_hex", v, sizeof v); assert(!strcmp(v, "#1E88E5"));
    // Profile 2: new name and colour, standard background, photo and flags kept.
    e = js_prox(f); f = js_fim(e);
    assert((int)js_num(e, f, "profile_index", 0) == 2);
    v[0] = 0; js_texto(e, f, "name", v, sizeof v); assert(!strcmp(v, "Kiddo"));
    v[0] = 0; js_texto(e, f, "avatar_color_hex", v, sizeof v); assert(!strcmp(v, "#E53935"));
    v[0] = 0; js_bruto(e, f, "profile_background_id", v, sizeof v); assert(!strcmp(v, "null"));
    v[0] = 0; js_bruto(e, f, "uses_primary_addons", v, sizeof v); assert(!strcmp(v, "true"));
    v[0] = 0; js_bruto(e, f, "avatar_url", v, sizeof v); assert(!strcmp(v, "null"));
    assert(!js_prox(f));
    free(b);
    b = perfiledit_corpo_push(pull, 1, "Main", 0, NULL, NULL, NULL, 2, "aurora", NULL);
    assert(b && strstr(b, "\"profile_background_id\":\"aurora\"") && !strstr(b, "p_origin_client_id"));
    free(b);
    // The link reaches the edited profile only; the others keep their photo.
    b = perfiledit_corpo_push(pull, 2, "Kid", 3, NULL, "https://img.example/k.png", NULL, 0, NULL, NULL);
    assert(b && strstr(b, "\"avatar_url\":\"https://img.example/k.png\"") && strstr(b, "\"avatar_url\":\"https://x/a1.png\""));
    free(b);
    // Refused whole: unknown profile, duplicates, ids out of range, more than 6.
    assert(!perfiledit_corpo_push(pull, 3, "X", 0, NULL, NULL, "#E53935", 0, NULL, NULL));
    assert(!perfiledit_corpo_push("[{\"profile_index\":1},{\"profile_index\":1}]", 1, "X", 0, NULL, NULL, NULL, 1, NULL, NULL));
    assert(!perfiledit_corpo_push("[{\"profile_index\":9}]", 1, "X", 0, NULL, NULL, NULL, 1, NULL, NULL));
    assert(!perfiledit_corpo_push("[{\"profile_index\":1},{\"profile_index\":2},{\"profile_index\":3},"
                                  "{\"profile_index\":4},{\"profile_index\":5},{\"profile_index\":6},"
                                  "{\"profile_index\":7}]", 1, "X", 0, NULL, NULL, NULL, 1, NULL, NULL));
    assert(!perfiledit_corpo_push(pull, 1, "X", 0, NULL, NULL, "#XYZ", 0, NULL, NULL));
  }
}

int main(void) {
  nomes(); regras(); catalogos(); corpos();
  puts("perfiledit: ok");
  return 0;
}
