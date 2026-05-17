#!/usr/bin/env python3
"""Simple HTTP server to serve SVG files from the graphs directory."""

import http.server
import socketserver
import os
import socket
import html

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

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
        lines.append("body { font-family: monospace; margin: 30px; background: #fff; color: #000; font-size: 20px; }")
        lines.append("a { color: #000; text-decoration: none; }")
        lines.append("a:hover { text-decoration: underline; }")
        lines.append("</style></head>")
        lines.append("<body><h1>Graphs</h1>")
        lines.append("<p>Total: {} SVG files</p><ul>".format(len(svg_files)))
        for f in svg_files:
            lines.append("<li><a href=\"/{}\">{}</a></li>".format(html.escape(f), html.escape(f)))
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
