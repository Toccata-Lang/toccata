#!/usr/bin/env python3
"""Simple HTTP server to serve SVG files from the graphs directory."""

import http.server
import socketserver
import os
import socket
import html
import json

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

TAG_MAP = {
    0x00: "VAL",   # positive native value
    0x01: "VAR",   # positive variable
    0x02: "SUB",   # negative possible deferred redex {- +}
    0x03: "NUL",   # positive eraser
    0x04: "ERA",   # negative eraser
    0x05: "LAM",   # positive constructor {- +}
    0x06: "APP",   # negative constructor {+ -}
    0x07: "REF",   # positive reference
    0x08: "VL1",   # positive native value alias
    0x09: "SUP",   # positive duplicator {+ +}
    0x0a: "DUP",   # negative duplicator {- -}
    0x0b: "OPX",   # negative operation {+ -}
    0x0c: "OPY",   # negative operation {+ -}
    0x0d: "I60",   # positive 56 bit int
    0x0e: "F60",   # positive 56 bit float
    0x0f: "LAZ",   # positive lazy node {- +}
}

def tag_to_str(tag):
    return TAG_MAP.get(tag, "UNKNOWN")

TAG_SIZE = 4
LAB_MASK = 0xFFFFFFF
LOC_SHIFT = TAG_SIZE + 28

def display(term):
    tag = term & 0xF
    lab = (term >> TAG_SIZE) & LAB_MASK
    loc = term >> LOC_SHIFT
    return f"{tag_to_str(tag)} {lab:03x} {loc:03x}"

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def log_message(self, format, *args):
        pass  # Suppress logs for cleaner output

    def do_GET(self):
        if self.path == "/" or self.path == "":
            self.list_svg()
            return
        if self.path.startswith("/interaction"):
            self.serve_interaction()
            return
        if self.path.startswith("/viewer"):
            self.show_svg_viewer()
            return
        if self.path.endswith(".svg"):
            self.serve_raw_svg()
            return
        self.send_error(404, "Not found")

    def get_svg_list(self):
        return sorted([f for f in os.listdir(DIRECTORY) if f.endswith(".svg")])

    def show_svg_viewer(self):
        svg_file = self.path.lstrip("/")
        svg_files = self.get_svg_list()
        try:
            idx = svg_files.index(svg_file)
        except ValueError:
            self.send_error(404, "Not found")
            return
        prev_file = svg_files[idx - 1] if idx > 0 else None
        next_file = svg_files[idx + 1] if idx < len(svg_files) - 1 else None

        self.send_response(200)
        self.send_header("Content-type", "text/html; charset=utf-8")
        self.end_headers()
        lines = []
        lines.append("<!DOCTYPE html>")
        lines.append("<html lang=\"en\">")
        lines.append("<head><meta charset=\"utf-8\"><title>Graph: {}</title>".format(html.escape(svg_file)))
        lines.append("<style>")
        lines.append("body { font-family: monospace; margin: 0; background: #fff; color: #222; display: flex; flex-direction: column; height: 100vh; }")
        lines.append(".nav { padding: 10px 20px; border-bottom: 1px solid #ccc; display: flex; justify-content: flex-start; align-items: center; gap: 10px; }")
        lines.append(".nav a { color: #222; text-decoration: none; font-size: 18px; padding: 5px 10px; border: 1px solid #ccc; border-radius: 4px; }")
        lines.append(".nav a.home { font-size: 14px; padding: 5px 8px; }")
        lines.append(".nav a:hover { background: #eee; }")
        lines.append(".nav .filename { font-size: 14px; color: #666; margin-left: auto; }")
        lines.append(".svg-container { flex: 1; padding: 20px; overflow: auto; }")
        lines.append(".svg-container svg { display: block; }")
        lines.append("</style></head>")
        lines.append("<body>")
        lines.append("<div class=\"nav\">")
        lines.append("<a href=\"/\" class=\"home\">&larr; Home</a>")
        if prev_file:
            lines.append("<a class=\"nav-btn\" onclick=\"loadSvg('/{}')\">&larr; Prev</a>".format(html.escape(prev_file)))
        if next_file:
            lines.append("<a class=\"nav-btn\" onclick=\"loadSvg('/{}')\">Next &rarr;</a>".format(html.escape(next_file)))
        lines.append("<span class=\"filename\" id=\"filename\">{}/{}: {}</span>".format(idx + 1, len(svg_files), html.escape(svg_file)))
        lines.append("</div>")
        lines.append("<div class=\"svg-container\">")
        lines.append('<img id="svg-img" src=\"/{}\" style=\"display: block;\">'.format(html.escape(svg_file)))
        lines.append("</div></body></html>")
        lines.append("<script>")
        lines.append("function loadSvg(url) {")
        lines.append("  var img = document.getElementById('svg-img');")
        lines.append("  img.src = url + '?t=' + Date.now();")
        lines.append("  var name = url.split('/').pop();")
        lines.append("  document.getElementById('filename').textContent = name;")
        lines.append("}")
        lines.append("</script>")
        self.wfile.write("\n".join(lines).encode())

    def serve_raw_svg(self):
        svg_file = self.path.lstrip("/")
        svg_path = os.path.join(DIRECTORY, svg_file)
        try:
            with open(svg_path, "r") as f:
                svg_content = f.read()
            self.send_response(200)
            self.send_header("Content-type", "image/svg+xml")
            self.send_header("Cache-Control", "no-cache")
            self.end_headers()
            self.wfile.write(svg_content.encode())
        except Exception:
            self.send_error(404, "Not found")

    def serve_interaction(self):
        from urllib.parse import urlparse, parse_qs
        parsed = urlparse(self.path)
        params = parse_qs(parsed.query)
        try:
            count = int(params.get("n", ["0"])[0])
        except ValueError:
            self.send_error(400, "Bad request: missing or invalid 'n' parameter")
            return
        svg_files = self.get_svg_list()
        target = "{}".format(count).zfill(4)
        matched = [f for f in svg_files if f.startswith(target + "-")]
        if not matched:
            self.send_error(404, "No SVG file for interaction {}".format(count))
            return
        svg_file = matched[0]
        svg_path = os.path.join(DIRECTORY, svg_file)
        try:
            with open(svg_path, "r") as f:
                svg_content = f.read()
            self.send_response(200)
            self.send_header("Content-type", "image/svg+xml")
            self.end_headers()
            self.wfile.write(svg_content.encode())
        except Exception:
            self.send_error(500, "Failed to read SVG")

    def list_svg(self):
        svg_files = sorted([f for f in os.listdir(DIRECTORY) if f.endswith(".svg")])
        self.send_response(200)
        self.send_header("Content-type", "text/html; charset=utf-8")
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate")
        self.end_headers()
        lines = []
        lines.append("<!DOCTYPE html>")
        lines.append("<html lang=\"en\">")
        lines.append("<head><meta charset=\"utf-8\"><title>Graphs</title>")
        lines.append("<meta http-equiv=\"Cache-Control\" content=\"no-store, no-cache, must-revalidate\">")
        lines.append("<style>")
        lines.append("body { margin: 0; font-family: monospace; background: #fff; color: #222; display: flex; height: 100vh; overflow: hidden; }")
        lines.append(".sidebar { width: 16%; min-width: 250px; border-right: 1px solid #ccc; overflow-y: auto; flex-shrink: 0; }")
        lines.append(".sidebar ul { list-style: none; margin: 0; padding: 0; }")
        lines.append(".sidebar li { padding: 4px 10px; cursor: pointer; font-size: 16px; border-bottom: 1px solid #eee; }")
        lines.append(".sidebar li:hover { background: #f5f5f5; }")
        lines.append(".sidebar li.active { background: #e0e0e0; font-weight: bold; }")
        lines.append(".main { flex: 1; display: flex; flex-direction: column; }")
        lines.append(".nav { padding: 8px 15px; border-bottom: 1px solid #ccc; display: flex; align-items: center; gap: 10px; }")
        lines.append(".nav a { color: #222; text-decoration: none; padding: 5px 10px; border: 1px solid #ccc; border-radius: 4px; cursor: pointer; font-size: 14px; }")
        lines.append(".nav a:hover { background: #eee; }")
        lines.append(".nav .filename { font-size: 13px; color: #666; margin-left: auto; }")
        lines.append(".svg-area { flex: 1; overflow: auto; padding: 10px; }")
        lines.append(".svg-area svg { display: block; }")
        lines.append("</style></head>")
        lines.append("<body>")
        lines.append("<div class=\"sidebar\">")
        lines.append("<ul id=\"file-list\">")
        for i, f in enumerate(svg_files):
            parts = f.rsplit(".", 1)
            name, ext = parts[0], parts[1] if len(parts) > 1 else ""
            dash_parts = name.split("-", 2)
            count = str(int(dash_parts[0])).rjust(4)
            neg = int(dash_parts[1]) if len(dash_parts) > 1 and dash_parts[1] else 0
            pos = int(dash_parts[2]) if len(dash_parts) > 2 and dash_parts[2] else 0
            neg_str = display(neg)
            pos_str = display(pos)
            display_name = "{}: {} / {}".format(count, neg_str, pos_str)
            lines.append("<li data-idx=\"{}\">{}</li>".format(i, html.escape(display_name)))
        lines.append("</ul></div>")
        lines.append("<div class=\"main\">")
        lines.append("<div class=\"nav\">")
        lines.append("<a id=\"prev-btn\">&larr; Prev</a>")
        lines.append("<a id=\"next-btn\">Next &rarr;</a>")
        lines.append("<span class=\"filename\" id=\"filename\">{}</span>".format(html.escape(svg_files[0]) if svg_files else ""))
        lines.append("</div>")
        lines.append("<div class=\"svg-area\" id=\"svg-area\"></div>")
        lines.append("</div></body></html>")
        lines.append("<script>")
        lines.append("var files = {};".format(json.dumps(svg_files)))
        lines.append("var currentIdx = 0;")
        lines.append("function showFile(idx) {")
        lines.append("  if (idx < 0 || idx >= files.length) return;")
        lines.append("  currentIdx = idx;")
        lines.append("  var svg = document.getElementById('svg-area');")
        lines.append("  svg.innerHTML = '<img src=\"/' + files[idx] + '\" style=\"display: block;\">';")
        lines.append("  document.getElementById('filename').textContent = files[idx];")
        lines.append("  var items = document.querySelectorAll('.sidebar li');")
        lines.append("  items.forEach(function(el) { el.classList.remove('active'); });")
        lines.append("  if (items[idx]) { items[idx].classList.add('active'); items[idx].scrollIntoView({block: 'nearest'}); }")
        lines.append("}")
        lines.append("document.querySelectorAll('.sidebar li').forEach(function(el) {")
        lines.append("  el.addEventListener('click', function() { showFile(parseInt(this.dataset.idx)); });")
        lines.append("});")
        lines.append("document.getElementById('prev-btn').addEventListener('click', function() { showFile(currentIdx - 1); });")
        lines.append("document.getElementById('next-btn').addEventListener('click', function() { showFile(currentIdx + 1); });")
        lines.append("showFile(0);")
        lines.append("</script>")
        self.wfile.write("\n".join(lines).encode())

class ReuseAddrTCPServer(socketserver.TCPServer):
    allow_reuse_address = True
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        super().server_bind()

class WatchdogTCPServer(socketserver.ThreadingMixIn, ReuseAddrTCPServer):
    daemon_threads = True
    allow_reuse_address = True
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        super().server_bind()

def main():
    import time
    import threading
    last_mtime = os.path.getmtime(__file__)
    while True:
        try:
            with WatchdogTCPServer(("", PORT), Handler) as httpd:
                print(f"Serving SVG files on http://localhost:{PORT}")
                def watch():
                    nonlocal last_mtime
                    while True:
                        time.sleep(0.5)
                        mtime = os.path.getmtime(__file__)
                        if mtime > last_mtime:
                            last_mtime = mtime
                            httpd.shutdown()
                            break
                t = threading.Thread(target=watch, daemon=True)
                t.start()
                httpd.serve_forever(poll_interval=0.5)
                print("File changed, restarting...")
        except KeyboardInterrupt:
            print("\nShutting down...")
            break

if __name__ == "__main__":
    main()
