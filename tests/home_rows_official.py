"""Stage G against regular NuvioTV's Home, pinned commit 99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f.

`gen` writes the scenario for tests/home_rows_official.c, with the synced payload
built as HomeCatalogSyncSupport.buildHomeCatalogSyncPayload builds it. `check`
compares Better Nuvio's real fil_unir/fil_oculta output with a port of
HomeViewModelCatalogUtils.rebuildCatalogOrder + normalizeCollectionBoundaries and
the computedHomeRows assembly (pinToTop collections, disabled keys skipped).
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = json.loads((ROOT / 'tests/fixtures/settings-restoration/stage-g-official-home.json').read_text())
APP_ROWS = {'continue_watching', 'social_activity', 'last_session'}


def type_key(t):  # CatalogReference.catalogTypeKey
    t = t.strip()
    return t.lower() if t.lower() in ('movie', 'series', 'channel', 'tv') else t


def show_on_home(c):  # CatalogDescriptor.shouldShowOnHome
    if any(e.get('name', '').lower() == 'search' and e.get('isRequired') for e in c.get('extra', [])):
        return False
    return c['showInHome'] is True if 'showInHome' in c else True


def catalogs(fx):
    out = []
    for addon in fx['addons']:
        for c in addon['catalogs']:
            if show_on_home(c):
                key = f"{addon['id']}_{type_key(c['type'])}_{c['id']}"
                if key not in [k for k, *_ in out]:
                    out.append((key, addon['id'], c['type'].strip(), c['id']))
    return out


def payload(fx):  # buildHomeCatalogSyncPayload on the regular Nuvio device
    cats = [c for c in catalogs(fx) if c[0] not in fx['unsaved_in_remote']]
    colls = [f"collection_{c['id']}" for c in fx['collections']]
    entries = {c[0]: c for c in cats}
    valid = [k for k in fx['remote_order'] if k in entries or k in colls]
    merged = valid + [c[0] for c in cats if c[0] not in valid] + [k for k in colls if k not in valid]
    items = []
    for i, k in enumerate(merged):
        enabled = k not in fx['remote_disabled']
        if k.startswith('collection_'):
            items.append({'addon_id': '', 'type': '', 'catalog_id': '', 'enabled': enabled, 'order': i,
                          'custom_title': '', 'is_collection': True, 'collection_id': k[11:]})
        else:
            _, addon, raw, cid = entries[k]
            items.append({'addon_id': addon, 'type': raw, 'catalog_id': cid, 'enabled': enabled, 'order': i,
                          'custom_title': '', 'is_collection': False, 'collection_id': ''})
    # The server returns the row; item order in the array is not guaranteed.
    items.reverse()
    return {'hide_unreleased_content': False, 'items': items}


def official_home(fx, follow):
    disabled = set(fx['remote_disabled'])
    pay = sorted(payload(fx)['items'], key=lambda i: i['order'])
    order_keys = [f"collection_{i['collection_id']}" if i['is_collection']
                  else f"{i['addon_id']}_{type_key(i['type'])}_{i['catalog_id']}" for i in pay]
    owners = {}
    for addon in fx['addons']:
        for c in addon['catalogs']:
            owners[f"{addon['id']}_{type_key(c['type'])}_{c['id']}"] = addon['id']
    default = [k for k, *_ in catalogs(fx) if k not in disabled]
    colls = [f"collection_{c['id']}" for c in fx['collections']]
    available = set(default + colls)
    saved = []
    for k in order_keys:
        if k in available and k not in saved:
            saved.append(k)
    if follow:
        if saved:
            result, p = [], 0
            for k in saved:
                if k in colls:
                    result.append(k)
                elif k in default:
                    t = default.index(k)
                    while p <= t:
                        if default[p] not in result:
                            result.append(default[p])
                        p += 1
            result += [k for k in default[p:] if k not in result]
            result += [k for k in colls if k not in result]
            changed = True
            while changed:  # normalizeCollectionBoundaries
                changed = False
                i = 0
                while i < len(result):
                    if not result[i].startswith('collection_'):
                        i += 1
                        continue
                    before = next((owners.get(result[j]) for j in range(i - 1, -1, -1)
                                   if not result[j].startswith('collection_')), None)
                    after = next((owners.get(result[j]) for j in range(i + 1, len(result))
                                  if not result[j].startswith('collection_')), None)
                    if before is not None and before == after:
                        key = result.pop(i)
                        pos = i
                        while pos < len(result) and not result[pos].startswith('collection_') and owners.get(result[pos]) == before:
                            pos += 1
                        result.insert(pos, key)
                        if pos != i:
                            changed = True
                    i += 1
            ordered = result
        else:
            ordered = default + colls
    else:
        ordered = saved + [k for k in default if k not in saved] + [k for k in colls if k not in saved]
    rows, added = [], set()
    for c in fx['collections']:  # computedHomeRows: pinned collections first
        k = f"collection_{c['id']}"
        if c['pinToTop'] and k not in disabled and k not in added:
            rows.append(k); added.add(k)
    for k in ordered:
        if k in disabled or k in added:
            continue
        if k.startswith('collection_'):
            added.add(k)
        rows.append(k)
    return rows


def gen(path, follow, variant):
    fx = dict(FIXTURE)
    native = [list(r) for r in fx['native_rows']]
    if variant == 'personal':  # an explicit TV hide on an unlisted shared row
        native = [[k, 1 if k == 'tmdb-addon_movie_tmdb.new_this_week' else o] for k, o in native]
    lines = [f"follow\t{follow}"]
    for i, a in enumerate(fx['addons']):
        lines.append(f"addon\t{a['id']}\t{a['base']}")
        for c in a['catalogs']:
            lines.append(f"cat\t{i}\t{json.dumps(c, separators=(',', ':'))}")
    for c in fx['collections']:
        lines.append(f"coll\t{c['id']}\t{1 if c['pinToTop'] else 0}")
    for k, o in native:
        lines.append(f"native\t{k}\t{o}")
    for k in fx['collection_members']:
        lines.append(f"member\t{k}")
    lines.append('payload\t' + json.dumps([{'profile_id': 1, 'settings_json': payload(fx),
                                            'updated_at': '2026-11-04T12:00:00+00:00'}], separators=(',', ':')))
    Path(path).write_text('\n'.join(lines) + '\n')


def check(actual_path, follow, variant):
    fx = FIXTURE
    actual = [l.split('\t', 1)[1] for l in Path(actual_path).read_text().splitlines() if l.startswith('ROW\t')]
    expected = official_home(fx, follow == '1')
    if variant == 'personal':
        expected = [k for k in expected if k != 'tmdb-addon_movie_tmdb.new_this_week']
    shared = [k for k in actual if k not in APP_ROWS]
    assert shared == expected, f"\nBetter Nuvio: {shared}\nregular Nuvio: {expected}"
    assert actual[0] == 'continue_watching', actual
    # Native-exclusive rows follow the shared row they followed natively.
    native = [k for k, _ in fx['native_rows']]
    for ex in ('social_activity', 'last_session'):
        anchor = next(k for k in reversed(native[:native.index(ex)]) if k not in APP_ROWS)
        i, j = actual.index(anchor), actual.index(ex)
        assert j > i and all(k in APP_ROWS for k in actual[i + 1:j]), (ex, anchor, actual)
    assert 'org.aiometadata.fixture_movie_streaming.netflix' not in actual  # showInHome:false
    assert 'org.aiometadata.fixture_Anime_mal.top' in actual                 # raw-case unknown type
    assert 'tmdb-addon_documentary_tmdb.docs' in actual                      # type longer than 7 bytes
    assert 'tmdb-addon_movie_tmdb.popular' in actual                         # "Movie" normalised
    print(f"PASS: follow_addons_order={follow} {variant}: {len(shared)} shared rows match regular Nuvio exactly; "
          "Continue Watching first, Friends Watching/Last Session anchored, showInHome/search/identity rules")


if __name__ == '__main__':
    {'gen': gen, 'check': check}[sys.argv[1]](*sys.argv[2:])
