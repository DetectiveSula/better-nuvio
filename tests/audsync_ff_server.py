#!/usr/bin/env python3
"""Range HTTP origin for tests/audsync_ff.sh: media files demand the playing
URL's headers (X-Fixture), every media request is logged as one JSON line."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import json, re, sys, threading
root, port_file, log_file = map(Path, sys.argv[1:])
lock = threading.Lock()
class Handler(BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    def log_message(self, *_): pass
    def do_HEAD(self): self.do_GET(head=True)
    def do_GET(self, head=False):
        target = root / self.path.lstrip('/').split('?')[0]
        media = target.suffix in ('.mkv', '.mp4')
        if media:
            with lock, log_file.open('a') as log:
                log.write(json.dumps({'path': self.path, 'range': self.headers.get('Range', ''),
                                      'fixture': self.headers.get('X-Fixture', '')}) + '\n')
            if self.headers.get('X-Fixture') != 'yes':
                self.send_response(403); self.send_header('Content-Length', '0'); self.end_headers(); return
        if target.parent != root or not target.is_file():
            self.send_response(404); self.send_header('Content-Length', '0'); self.end_headers(); return
        total = target.stat().st_size
        lo, hi, code = 0, total - 1, 200
        m = re.fullmatch(r'bytes=(\d+)-(\d*)', self.headers.get('Range', ''))
        if m:
            lo = int(m.group(1)); hi = min(int(m.group(2)) if m.group(2) else total - 1, total - 1); code = 206
            if lo >= total:
                self.send_response(416); self.send_header('Content-Range', f'bytes */{total}')
                self.send_header('Content-Length', '0'); self.end_headers(); return
        self.send_response(code)
        self.send_header('Accept-Ranges', 'bytes')
        if code == 206: self.send_header('Content-Range', f'bytes {lo}-{hi}/{total}')
        self.send_header('Content-Length', str(hi - lo + 1)); self.end_headers()
        if head: return
        try:
            with target.open('rb') as f:
                f.seek(lo); left = hi - lo + 1
                while left > 0:
                    chunk = f.read(min(left, 1 << 16))
                    if not chunk: break
                    self.wfile.write(chunk); left -= len(chunk)
        except (BrokenPipeError, ConnectionResetError):
            pass
server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
server.daemon_threads = True
server.handle_error = lambda *_: None   # client resets on cancel are expected
port_file.write_text(str(server.server_address[1]))
server.serve_forever()
