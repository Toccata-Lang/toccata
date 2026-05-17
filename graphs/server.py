#!/usr/bin/env python3
"""Simple HTTP server to serve SVG files from the graphs directory."""

import http.server
import socketserver
import os
import socket
import html

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
        if not self.path.endswith(".svg"):
            self.send_error(404, "Not found")
            return
        super().do_GET()

    def list_svg(self):
        svg_files = sorted([f for f in os.listdir(DIRECTORY) if f.endswith(".svg")])
        self.send_response(200)
        self.send_header("Content-type", "text/html; charset=utf-8")
        self.end_headers()
        lines = []
        lines.append("<!DOCTYPE html>")
        lines.append("<html lang=\"en\">")
        lines.append("<head><meta charset=\"utf-8\"><title>Graphs</title>")
        lines.append("<style>")
        lines.append("body { font-family: monospace; margin: 30px; background: #fff; color: #222; font-size: 20px; }")
        lines.append("a { color: #222; text-decoration: none; }")
        lines.append("a:hover { text-decoration: underline; }")
        lines.append("</style></head>")
        lines.append("<body><h1>Graphs</h1>")
        lines.append("<p>Total: {} SVG files</p><ul>".format(len(svg_files)))
        for f in svg_files:
            parts = f.rsplit(".", 1)
            name, ext = parts[0], parts[1] if len(parts) > 1 else ""
            dash_parts = name.split("-", 2)
            count = str(int(dash_parts[0])).rjust(4)
            neg = int(dash_parts[1], 16) if len(dash_parts) > 1 and dash_parts[1] else 0
            pos = int(dash_parts[2], 16) if len(dash_parts) > 2 and dash_parts[2] else 0
            neg_str = display(neg)
            pos_str = display(pos)
            lines.append("<li><a href=\"/{}\">{}: {} / {}</a></li>".format(html.escape(f), html.escape(count), html.escape(neg_str), html.escape(pos_str)))
        lines.append("</ul></body></html>")
        self.wfile.write("\n".join(lines).encode())

class ReuseAddrTCPServer(socketserver.TCPServer):
    allow_reuse_address = True
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        super().server_bind()

if __name__ == "__main__":
    with ReuseAddrTCPServer(("", PORT), Handler) as httpd:
        print(f"Serving SVG files on http://localhost:{PORT}")
        httpd.serve_forever()
