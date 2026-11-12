// PROFILE FRAMES (molduras): draws the original frame artwork procedurally, so
// no third-party art enters the package. Each frame is a 512x512 PNG with a
// transparent centre: the avatar circle (radius AV_R) shows through, and the
// frame is drawn over it at FRAME_SCALE x the avatar's diameter.
//
//   cc tools/molduras.c -I/opt/homebrew/include -L/opt/homebrew/lib \
//      -lSDL2 -lSDL2_image -lSDL2_ttf -lm -o /tmp/molduras
//   /tmp/molduras <out-dir> [avatar.jpg font.ttf [set]]   (contact sheet of set 0 rings / 1 decorations)
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N      512
#define C      256.0
#define AV_R   200.0          // avatar radius inside the 512 canvas
#define SS     4              // supersamples per axis
#define PI     3.14159265358979

typedef struct { double r, g, b, a; } Px;   // straight alpha, 0..1

static Px over(Px dst, Px src) {             // src over dst
  double a = src.a + dst.a * (1 - src.a);
  if (a <= 0) return (Px){0, 0, 0, 0};
  return (Px){(src.r * src.a + dst.r * dst.a * (1 - src.a)) / a,
              (src.g * src.a + dst.g * dst.a * (1 - src.a)) / a,
              (src.b * src.a + dst.b * dst.a * (1 - src.a)) / a, a};
}
static Px rgb(int r, int g, int b, double a) { return (Px){r / 255.0, g / 255.0, b / 255.0, a}; }
static Px mix(Px p, Px q, double t) {
  return (Px){p.r + (q.r - p.r) * t, p.g + (q.g - p.g) * t, p.b + (q.b - p.b) * t, p.a + (q.a - p.a) * t};
}
static double clamp01(double v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
static double ang(double x, double y) { double a = atan2(x, -y); return a < 0 ? a + 2 * PI : a; } // 0 at top, clockwise
// Rotate (x,y) into the frame of a point at angle `a` on radius `rad`: u along
// the tangent, v outward.
static void local(double x, double y, double a, double rad, double *u, double *v) {
  double cx = sin(a) * rad, cy = -cos(a) * rad, dx = x - cx, dy = y - cy;
  *u = dx * cos(a) + dy * sin(a);
  *v = dx * sin(a) - dy * cos(a);
}
static double nearestSlot(double a, int count) { double s = 2 * PI / count; return floor(a / s + 0.5) * s; }
static int ring(double d, double r0, double r1) { return d >= r0 && d <= r1; }
// Soft falloff for glows, computed per sample (supersampling smooths the rest).
static double glow(double d, double r, double w) { double t = fabs(d - r) / w; return exp(-t * t * 2.5); }

// ---- the frames ----------------------------------------------------------------
// Bevelled metal: a light-to-dark ramp across the band, brightest at the top-left.
static Px metal(Px base, double d, double r0, double r1, double x, double y) {
  double t = (d - r0) / (r1 - r0), light = 0.75 + 0.35 * cos(ang(x, y) + PI * 0.25);
  double k = (0.55 + 0.45 * sin(t * PI)) * light;
  return (Px){clamp01(base.r * k + 0.25 * (1 - t) * light), clamp01(base.g * k + 0.22 * (1 - t) * light),
              clamp01(base.b * k + 0.12 * (1 - t) * light), 1};
}

static Px laurel(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px p = {0}, gold = rgb(224, 178, 72, 1), deep = rgb(150, 104, 30, 1);
  if (ring(d, AV_R - 4, AV_R + 10)) p = metal(gold, d, AV_R - 4, AV_R + 10, x, y);
  // Two branches meeting at the bottom, open at the top: leaves alternate in/out.
  if (a > 0.32 && a < 2 * PI - 0.32 && fabs(a - PI) > 0.10) {
    for (int side = 0; side < 2; side++) {
      double step = 0.21;
      double aa = nearestSlot(a - side * step * 0.5, (int)(2 * PI / step)) + side * step * 0.5;
      double rad = AV_R + (side ? 34 : 24), u, v;
      local(x, y, aa, rad, &u, &v);
      double tilt = (aa < PI ? 1 : -1) * 0.55, uu = u * cos(tilt) - v * sin(tilt), vv = u * sin(tilt) + v * cos(tilt);
      double e = (uu * uu) / (19.0 * 19.0) + (vv * vv) / (8.0 * 8.0);
      if (e <= 1) p = over(p, mix(gold, deep, clamp01(e * 0.8 + (vv > 0 ? 0.25 : 0))));
      else if (e <= 1.25) p = over(p, (Px){deep.r, deep.g, deep.b, 0.85});
    }
  }
  // Ribbon knot at the bottom.
  { double u, v; local(x, y, PI, AV_R + 30, &u, &v);
    if (fabs(u) < 14 && fabs(v) < 12) p = over(p, rgb(176, 40, 52, 1));
    if (fabs(u) < 6 && fabs(v) < 13) p = over(p, rgb(120, 24, 36, 1)); }
  return p;
}

static Px neon(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  // Hue sweeps cyan -> magenta -> cyan around the ring.
  double t = 0.5 - 0.5 * cos(a);
  Px hue = mix(rgb(40, 240, 255, 1), rgb(255, 60, 220, 1), t);
  Px p = {0};
  double g = glow(d, AV_R + 10, 26);
  p = over(p, (Px){hue.r, hue.g, hue.b, 0.75 * g});
  if (ring(d, AV_R + 5, AV_R + 15)) p = over(p, mix(hue, rgb(255, 255, 255, 1), 0.55 * clamp01(1 - fabs(d - AV_R - 10) / 5)));
  if (ring(d, AV_R - 3, AV_R + 1)) p = over(p, (Px){hue.r, hue.g, hue.b, 0.9});
  return p;
}

static Px film(double x, double y) {
  double d = hypot(x, y), a = ang(x, y), r0 = AV_R - 2, r1 = AV_R + 46;
  Px p = {0};
  if (!ring(d, r0, r1)) return p;
  p = rgb(24, 24, 28, 1);
  if (ring(d, r0, r0 + 3) || ring(d, r1 - 3, r1)) p = rgb(70, 70, 78, 1);
  // Sprocket holes on both edges.
  for (int edge = 0; edge < 2; edge++) {
    double rad = edge ? r1 - 11 : r0 + 11, u, v;
    double aa = nearestSlot(a, edge ? 44 : 36);
    local(x, y, aa, rad, &u, &v);
    // Lit holes, not transparent ones: on the dark picker a see-through hole vanishes.
    if (fabs(u) < 6.5 && fabs(v) < 4.5) p = mix(rgb(250, 244, 226, 1), rgb(200, 190, 160, 1), clamp01((fabs(u) + fabs(v)) / 11));
  }
  // Frame dividers across the middle.
  { double u, v; local(x, y, nearestSlot(a, 12), (r0 + r1) / 2, &u, &v);
    if (fabs(u) < 1.6 && fabs(v) < 9) p = rgb(90, 90, 100, 1); }
  return p;
}

static Px frost(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px ice = rgb(190, 232, 255, 1), deep = rgb(96, 170, 230, 1), p = {0};
  p = over(p, (Px){ice.r, ice.g, ice.b, 0.35 * glow(d, AV_R + 6, 18)});
  if (ring(d, AV_R - 3, AV_R + 9)) p = over(p, mix(deep, ice, clamp01((d - AV_R + 3) / 12)));
  // Six snowflakes; small sparkles between them.
  double aa = nearestSlot(a, 6), u, v;
  local(x, y, aa, AV_R + 28, &u, &v);
  double rr = hypot(u, v), th = atan2(v, u);
  if (rr < 24) {
    for (int k = 0; k < 6; k++) {
      double t = th - k * PI / 3, along = rr * cos(t), across = fabs(rr * sin(t));
      if (along > 0 && across < 2.2) p = over(p, ice);
      // side barbs
      for (int b = 1; b <= 2; b++) {
        double bx = along - b * 7.5, by = rr * sin(t);
        double bt1 = fabs(bx * sin(PI / 3) - fabs(by) * cos(PI / 3));
        if (bx < 0 && bx > -6 && bt1 < 1.8 && along > 0) p = over(p, ice);
      }
    }
    if (rr < 4) p = over(p, rgb(255, 255, 255, 1));
  }
  { double a2 = nearestSlot(a - PI / 6, 6) + PI / 6, u2, v2; local(x, y, a2, AV_R + 22, &u2, &v2);
    if (fabs(u2) * fabs(v2) < 6 && fabs(u2) < 9 && fabs(v2) < 9) p = over(p, rgb(240, 250, 255, 1)); }
  return p;
}

static Px starburst(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px gold = rgb(255, 196, 64, 1), p = {0};
  // 24 rays of alternating length radiate from the ring.
  double s = 2 * PI / 24, k = a / s - floor(a / s), tri = 1 - fabs(k - 0.5) * 2;   // 1 at the ray centre
  int longRay = ((int)floor(a / s)) % 2 == 0;
  double tip = AV_R + (longRay ? 52 : 32);
  if (d > AV_R && d < tip && tri > (d - AV_R) / (tip - AV_R)) p = mix(rgb(255, 236, 160, 1), gold, (d - AV_R) / (tip - AV_R));
  if (ring(d, AV_R - 4, AV_R + 8)) p = metal(gold, d, AV_R - 4, AV_R + 8, x, y);
  return p;
}

static Px aurora(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  // Wavy band: its outer edge undulates around the ring.
  double outer = AV_R + 22 + 9 * sin(a * 5) + 5 * sin(a * 11 + 1.3);
  Px c1 = rgb(60, 240, 160, 1), c2 = rgb(40, 200, 220, 1), c3 = rgb(160, 90, 255, 1);
  double t = 0.5 + 0.5 * sin(a * 2 + 0.8);
  Px hue = t < 0.5 ? mix(c1, c2, t * 2) : mix(c2, c3, (t - 0.5) * 2), p = {0};
  p = over(p, (Px){hue.r, hue.g, hue.b, 0.45 * glow(d, outer, 22)});
  if (d > AV_R - 3 && d < outer) p = over(p, (Px){hue.r, hue.g, hue.b, 0.55 + 0.45 * (1 - (d - AV_R) / (outer - AV_R))});
  if (ring(d, AV_R - 3, AV_R + 1)) p = over(p, rgb(235, 255, 250, 1));
  return p;
}

static Px royal(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px gold = rgb(214, 170, 76, 1), p = {0};
  if (ring(d, AV_R - 3, AV_R + 7)) p = metal(gold, d, AV_R - 3, AV_R + 7, x, y);
  if (ring(d, AV_R + 17, AV_R + 23)) p = metal(gold, d, AV_R + 17, AV_R + 23, x, y);
  // Eight gems between the rings: ruby at the cardinal points, sapphire between.
  double aa = nearestSlot(a, 8), u, v; local(x, y, aa, AV_R + 15, &u, &v);
  int slot = (int)lround(aa / (PI / 4)) % 8;
  Px gem = slot % 2 ? rgb(60, 110, 230, 1) : rgb(210, 36, 64, 1);
  double r = hypot(u, v);
  if (r < 13) p = over(p, gold);
  if (r < 10) p = over(p, mix(gem, rgb(255, 255, 255, 1), clamp01(0.7 - (u + v + 10) / 20)));
  // Crown at the top.
  { double u2, v2; local(x, y, 0, AV_R + 40, &u2, &v2);
    if (v2 > -12 && v2 < 4 && fabs(u2) < 30) p = over(p, metal(gold, v2 + 12, 0, 16, x, y));
    for (int k = -1; k <= 1; k++) {
      double px = u2 - k * 22, top = 22 - fabs(px) * 1.6;
      if (v2 >= 4 && v2 < top && fabs(px) < 10) p = over(p, metal(gold, v2, 4, 22, x, y));
      if (hypot(px, v2 - 24) < 4.5) p = over(p, rgb(240, 240, 255, 1));
    } }
  return p;
}

static Px petals(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px pink = rgb(255, 150, 190, 1), deep = rgb(220, 80, 140, 1), p = {0};
  // Two rows of overlapping petals (circles), offset by half a slot.
  for (int row = 1; row >= 0; row--) {
    int count = 20;
    double off = row ? PI / count : 0, aa = nearestSlot(a - off, count) + off, u, v;
    for (int n = -1; n <= 1; n++) {
      double ab = aa + n * 2 * PI / count;
      local(x, y, ab, AV_R + (row ? 26 : 16), &u, &v);
      double r = hypot(u, v * 1.15);
      if (r < 21) p = over(p, mix(row ? deep : pink, rgb(255, 235, 245, 1), clamp01(0.55 - v / 40 - r / 60)));
      else if (r < 22.5) p = over(p, (Px){deep.r, deep.g, deep.b, 0.8});
    }
  }
  if (d < AV_R) p = (Px){0, 0, 0, 0};       // never over the face
  if (ring(d, AV_R - 3, AV_R + 2)) p = over(p, rgb(255, 245, 250, 1));
  return p;
}


// ---- decorations: Discord-style, they break out of the circle ------------------
// Same canvas and avatar radius as the rings, but they may overlap the avatar's
// edge (ears, cups) down to CLEAR_DECOR; the face itself stays clear.
#define CLEAR_DECOR (AV_R - 52)
static int inTri(double px, double py, double ax, double ay, double bx, double by, double cx, double cy) {
  double d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
  double d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
  double d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);
  int neg = d1 < 0 || d2 < 0 || d3 < 0, pos = d1 > 0 || d2 > 0 || d3 > 0;
  return !(neg && pos);
}
static Px catEars(double x, double y) {
  Px p = {0}, fur = rgb(243, 238, 232, 1), inner = rgb(255, 150, 186, 1), line = rgb(43, 39, 48, 1);
  for (int s = -1; s <= 1; s += 2) {
    double u, v; local(x, y, s * 0.60, AV_R - 46, &u, &v);
    double tipU = s * 10.0;
    for (int layer = 0; layer < 3; layer++) {
      double k = layer == 0 ? 1.06 : layer == 1 ? 1.0 : 0.58, lift = layer == 2 ? 22 : 0, H = 104;
      double cu = tipU / 3, cv = H / 3;
      double ax = cu + (-60 - cu) * k, ay = cv + (0 - cv) * k + lift, bx = cu + (60 - cu) * k, by = ay;
      double tx = cu + (tipU - cu) * k, ty = cv + (H - cv) * k + lift * 0.4;
      if (inTri(u, v, ax, ay, bx, by, tx, ty)) p = over(p, layer == 0 ? line : layer == 1 ? fur : mix(inner, fur, clamp01((v - 30) / 100)));
    }
  }
  return p;
}
static Px headphones(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  Px p = {0}, body = rgb(44, 47, 56, 1), hi = rgb(84, 90, 104, 1), teal = rgb(45, 212, 191, 1);
  double top = a > PI ? a - 2 * PI : a;                      // -PI..PI, 0 at the top
  if (fabs(top) < 1.42 && ring(d, AV_R + 18, AV_R + 36)) {
    p = d < AV_R + 22 || d > AV_R + 32 ? body : hi;
    if (ring(d, AV_R + 25.5, AV_R + 28.5) && fabs(top) < 1.2) p = teal;
  }
  for (int s = -1; s <= 1; s += 2) {
    double u, v; local(x, y, s * PI / 2, AV_R + 4, &u, &v);
    double qu = fabs(u) - 30, qv = fabs(v) - 34, r = 14;
    double sd = hypot(fmax(qu, 0), fmax(qv, 0)) + fmin(fmax(qu, qv), 0) - r;
    if (sd < 0) p = over(p, body);
    if (sd < -3 && v < -6) p = over(p, mix(hi, body, clamp01((-v - 6) / 30)));      // cushion, toward the face
    if (hypot(u, v - 22) < 5) p = over(p, teal);                                      // LED
  }
  return p;
}
static Px flames(double x, double y) {
  double d = hypot(x, y), a = ang(x, y);
  double top = 0.55 + 0.45 * cos(a);                          // taller over the head
  double h = (16 + 40 * pow(fabs(sin(a * 7 + 0.8 * sin(a * 3))), 0.6)) * top + 10;
  Px p = {0};
  if (d < AV_R - 6 || d > AV_R + h) return p;
  double t = clamp01((d - AV_R + 6) / (h + 6));
  Px c = t < 0.45 ? mix(rgb(255, 244, 150, 1), rgb(255, 150, 30, 1), t / 0.45) : mix(rgb(255, 150, 30, 1), rgb(220, 40, 30, 1), (t - 0.45) / 0.55);
  c.a = 1 - t * t * t;
  return over(p, c);
}
static Px halo(double x, double y) {
  Px p = {0}, gold = rgb(255, 214, 102, 1);
  double ex = x / 104.0, ey = (y + AV_R + 6) / 22.0, e = sqrt(ex * ex + ey * ey);
  p = over(p, (Px){1, 0.96, 0.8, 0.55 * glow(e, 1, 0.22)});
  if (fabs(e - 1) < 0.13) p = over(p, mix(rgb(255, 248, 210, 1), gold, clamp01((e - 0.87) / 0.26)));
  return p;
}
static Px horns(double x, double y) {
  Px p = {0}, dark = rgb(150, 22, 36, 1), bright = rgb(255, 92, 80, 1);
  for (int s = -1; s <= 1; s += 2) {
    double a0 = s * 0.55, bx = sin(a0) * (AV_R - 14), by = -cos(a0) * (AV_R - 14);
    double ox = sin(a0), oy = -cos(a0);                        // outward
    double tx = -oy * s, ty = ox * s;                          // tangent, toward the outside of the head
    double best = 1e9, bestT = 0, side = 0;
    for (int k = 0; k <= 40; k++) {
      double t = k / 40.0;
      // Rises outward, then hooks back over the head.
      double cx = bx + ox * (78 * t) + tx * (34 * t - 62 * t * t * t);
      double cy = by + oy * (78 * t) + ty * (34 * t - 62 * t * t * t);
      double r = 24 * (1 - t) + 2.5, dd = hypot(x - cx, y - cy) - r;
      if (dd < best) { best = dd; bestT = t; side = (x - cx) * tx + (y - cy) * ty; }
    }
    if (best < 2.5) p = over(p, rgb(40, 8, 14, 1));
    if (best < 0) p = over(p, mix(dark, bright, clamp01(0.5 + side / 30 + bestT * 0.2)));
  }
  return p;
}
static Px flowerCrown(double x, double y) {
  double a = ang(x, y), top = a > PI ? a - 2 * PI : a;
  Px p = {0};
  if (fabs(top) > 1.25) return p;
  // Leaves.
  { double aa = nearestSlot(a, 40), u, v; local(x, y, aa, AV_R + 6, &u, &v);
    int k = (int)lround(aa / (2 * PI / 40)); double tilt = k % 2 ? 0.7 : -0.7;
    double uu = u * cos(tilt) - v * sin(tilt), vv = u * sin(tilt) + v * cos(tilt);
    double atop = aa > PI ? aa - 2 * PI : aa;
    if (fabs(atop) < 1.15 && uu * uu / 256.0 + vv * vv / 36.0 < 1) p = over(p, mix(rgb(70, 160, 80, 1), rgb(140, 210, 110, 1), clamp01(0.5 + vv / 12))); }
  // Flowers.
  static const Px petal[] = {{1, 0.62, 0.78, 1}, {1, 1, 1, 1}, {1, 0.85, 0.35, 1}, {0.78, 0.65, 1, 1}};
  for (int n = -3; n <= 3; n++) {
    double aa = n * 0.34, u, v; local(x, y, aa, AV_R + 14 + (n % 2 ? 0 : 6), &u, &v);
    double r = hypot(u, v), th = atan2(v, u);
    double pr = 13 + 9 * fabs(cos(th * 2.5));                // five petals
    Px c = petal[(n + 4) % 4];
    if (r < pr) p = over(p, mix(c, rgb(255, 255, 255, 1), clamp01(0.4 - r / 40)));
    if (r < 6.5) p = over(p, n % 2 ? rgb(255, 200, 60, 1) : rgb(240, 120, 40, 1));
  }
  return p;
}
static Px sparkles(double x, double y) {
  static const double S[][3] = {{-0.9, 240, 22}, {-0.35, 238, 12}, {0.25, 244, 30}, {0.95, 232, 14},
                                {1.7, 240, 18}, {2.5, 236, 10}, {3.3, 242, 24}, {4.1, 236, 12}, {4.9, 240, 16}, {5.6, 236, 10}};
  Px p = {0}, white = rgb(255, 255, 255, 1), teal = rgb(150, 245, 230, 1);
  double d = hypot(x, y);
  p = over(p, (Px){0.6, 0.95, 0.9, 0.35 * glow(d, AV_R + 4, 10)});
  if (ring(d, AV_R - 2, AV_R + 2)) p = over(p, (Px){1, 1, 1, 0.85});
  for (unsigned k = 0; k < sizeof S / sizeof S[0]; k++) {
    double u, v; local(x, y, S[k][0], S[k][1], &u, &v);
    double sz = S[k][2], q = sqrt(fabs(u) / sz) + sqrt(fabs(v) / sz);
    p = over(p, (Px){teal.r, teal.g, teal.b, 0.5 * glow(hypot(u, v), 0, sz * 0.6)});
    if (q < 1) p = over(p, mix(white, teal, q));
  }
  return p;
}

typedef struct { const char *id, *name; Px (*fn)(double, double); double clear; int set; } Frame;
static const Frame FRAMES[] = {
  {"laurel", "Laurel", laurel, AV_R - 4, 0}, {"neon", "Neon", neon, AV_R - 4, 0}, {"film", "Film strip", film, AV_R - 4, 0},
  {"frost", "Frost", frost, AV_R - 4, 0}, {"starburst", "Starburst", starburst, AV_R - 4, 0}, {"aurora", "Aurora", aurora, AV_R - 4, 0},
  {"royal", "Royal", royal, AV_R - 4, 0}, {"petals", "Petals", petals, AV_R - 4, 0},
  {"cat-ears", "Cat ears", catEars, CLEAR_DECOR, 1}, {"headphones", "Headphones", headphones, CLEAR_DECOR, 1},
  {"flames", "Flames", flames, AV_R - 8, 1}, {"halo", "Halo", halo, AV_R - 4, 1}, {"horns", "Horns", horns, CLEAR_DECOR, 1},
  {"flower-crown", "Flower crown", flowerCrown, CLEAR_DECOR, 1}, {"sparkles", "Sparkles", sparkles, AV_R - 4, 1},
};
#define NF ((int)(sizeof FRAMES / sizeof FRAMES[0]))

static SDL_Surface *render(const Frame *f) {
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, N, N, 32, SDL_PIXELFORMAT_RGBA32);
  Uint8 *px = s->pixels;
  for (int j = 0; j < N; j++) for (int i = 0; i < N; i++) {
    double r = 0, g = 0, b = 0, a = 0;
    for (int sy = 0; sy < SS; sy++) for (int sx = 0; sx < SS; sx++) {
      double x = i + (sx + 0.5) / SS - C, y = j + (sy + 0.5) / SS - C;
      Px p = f->fn(x, y);
      if (hypot(x, y) < f->clear) p.a = 0;        // the face is always the avatar
      r += p.r * p.a; g += p.g * p.a; b += p.b * p.a; a += p.a;
    }
    Uint8 *o = px + j * s->pitch + i * 4;
    double n = SS * SS;
    o[3] = (Uint8)lround(clamp01(a / n) * 255);
    o[0] = a > 0 ? (Uint8)lround(clamp01(r / a) * 255) : 0;
    o[1] = a > 0 ? (Uint8)lround(clamp01(g / a) * 255) : 0;
    o[2] = a > 0 ? (Uint8)lround(clamp01(b / a) * 255) : 0;
  }
  return s;
}

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: molduras <out-dir> [avatar.jpg font.ttf]\n"); return 2; }
  SDL_Init(0); IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG); TTF_Init();
  SDL_Surface *frames[NF];
  char path[1024];
  for (int k = 0; k < NF; k++) {
    frames[k] = render(&FRAMES[k]);
    snprintf(path, sizeof path, "%s/moldura-%s.png", argv[1], FRAMES[k].id);
    if (IMG_SavePNG(frames[k], path)) { fprintf(stderr, "save failed: %s\n", path); return 1; }
    printf("%s\n", path);
  }
  if (argc < 4) return 0;
  // Contact sheet: each frame over the same sample avatar, on the picker's dark backdrop.
  SDL_Surface *poster = IMG_Load(argv[2]);
  TTF_Font *font = TTF_OpenFont(argv[3], 26);
  if (!poster || !font) { fprintf(stderr, "sheet inputs missing\n"); return 1; }
  SDL_Surface *av = SDL_ConvertSurfaceFormat(poster, SDL_PIXELFORMAT_RGBA32, 0);
  int set = argc > 4 ? atoi(argv[4]) : 0, list[NF], count = 0;   // 0 = rings, 1 = decorations
  for (int k = 0; k < NF; k++) if (FRAMES[k].set == set) list[count++] = k;
  const int T = 300, cols = 4, rows = (count + cols - 1) / cols, LABEL = 44;
  SDL_Surface *sheet = SDL_CreateRGBSurfaceWithFormat(0, T * cols, (T + LABEL) * rows, 32, SDL_PIXELFORMAT_RGBA32);
  SDL_FillRect(sheet, NULL, SDL_MapRGBA(sheet->format, 14, 16, 22, 255));
  double scale = (double)N / T;                    // tile pixel -> frame pixel
  for (int m = 0; m < count; m++) {
    int k = list[m], ox = (m % cols) * T, oy = (m / cols) * (T + LABEL);
    for (int j = 0; j < T; j++) for (int i = 0; i < T; i++) {
      double fx = (i + 0.5) * scale, fy = (j + 0.5) * scale, dx = fx - C, dy = fy - C;
      Uint8 *o = (Uint8 *)sheet->pixels + (oy + j) * sheet->pitch + (ox + i) * 4;
      double r = o[0] / 255.0, g = o[1] / 255.0, b = o[2] / 255.0;
      if (hypot(dx, dy) < AV_R) {                  // avatar: centre crop of the sample image
        int side = av->w < av->h ? av->w : av->h;
        int sx = (av->w - side) / 2 + (int)((dx + AV_R) / (2 * AV_R) * side);
        int sy = (av->h - side) / 2 + (int)((dy + AV_R) / (2 * AV_R) * side);
        Uint8 *q = (Uint8 *)av->pixels + sy * av->pitch + sx * 4;
        r = q[0] / 255.0; g = q[1] / 255.0; b = q[2] / 255.0;
      }
      int fi = (int)fx, fj = (int)fy;
      Uint8 *q = (Uint8 *)frames[k]->pixels + fj * frames[k]->pitch + fi * 4;
      double fa = q[3] / 255.0;
      r = r * (1 - fa) + q[0] / 255.0 * fa; g = g * (1 - fa) + q[1] / 255.0 * fa; b = b * (1 - fa) + q[2] / 255.0 * fa;
      o[0] = (Uint8)lround(r * 255); o[1] = (Uint8)lround(g * 255); o[2] = (Uint8)lround(b * 255); o[3] = 255;
    }
    SDL_Surface *t = TTF_RenderUTF8_Blended(font, FRAMES[k].name, (SDL_Color){235, 236, 240, 255});
    SDL_Rect dst = {ox + (T - t->w) / 2, oy + T + 4, t->w, t->h};
    SDL_BlitSurface(t, NULL, sheet, &dst); SDL_FreeSurface(t);
  }
  snprintf(path, sizeof path, "%s/sheet-%s.png", argv[1], set ? "decorations" : "rings");
  IMG_SavePNG(sheet, path); printf("%s\n", path);
  return 0;
}
