# Artwork arriving while scrolling

The Home's collection-cover branch skipped the ordinary horizontal preload
logic, its covers were excluded from row residency, and vertical lookahead
ran only after scrolling had settled. Horizontal lookahead also repeatedly
tried a single neighbour, even if it was already cached. Consequently the
next cover or row often started loading only when it became visible.

Home now resolves collection folder covers explicitly, retains them with the
nearby-row cache, prepares a bounded six-card horizontal strip, skips already
known neighbours, and prepares upcoming rows during vertical movement. It uses
the existing low-priority texture queue, request-rate limit, memory-pressure
checks and texture budget. Detail/immersive transitions do not start this work.

The texture lookahead admission gate also required an entirely empty slot and
counted completed lookahead textures forever. That stopped preloading in a
populated cache or after 30 unseen prefetched textures. Admission now permits
the existing cold LRU to recycle nonvisible, nonresident entries and counts
only unfinished speculative work. At most eight total jobs are admitted by
moving lookahead; visible/resident artwork and memory-pressure checks remain
protected.

Validation: `python3 tests/home_poster_prefetch.py`,
`TMPDIR=/tmp bash tests/posterfluido.sh`, `bash tests/home.sh`, and a syntax
check of the complete production Home source. The tests cover collection-cover
URLs, preloading before visibility, cached-neighbour skipping, scrolling in
both directions, full cache tables, more than 30 completed lookahead entries,
cold-cache reclamation, protected artwork and memory pressure. These are local
tests; the changes have not been installed or measured on the physical TV.

## Unexpected exit reported during scrolling

The connected S90C's restart log reports that the prior 1.0.7 session did not
exit cleanly after approximately 50 minutes. Its last recorded RSS was 93 MB.
The accessible log contains no terminating native signal, managed exception
or stack trace. The TV's system-log command returned no data. This confirms an
unclean prior session, but does not establish its cause or exclude a transient
memory spike. The current read-only log endpoint serves the new session, not
the private `nuvio-anterior.log` file needed to inspect the end of the previous
session. No crash cause or crash fix is claimed.
