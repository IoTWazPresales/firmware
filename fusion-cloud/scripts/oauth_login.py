#!/usr/bin/env python3
"""
One-time local OAuth login to obtain APS_REFRESH_TOKEN for cloud agent use.

Run on YOUR computer (not the cloud VM):
  cd fusion-cloud
  cp .env.example .env   # fill APS_CLIENT_ID and APS_CLIENT_SECRET
  uv sync
  uv run python scripts/oauth_login.py

Add http://localhost:8080/callback to your APS app's Callback URLs first.
"""

from __future__ import annotations

import os
import sys
import threading
import time
import urllib.parse
import webbrowser
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path

import httpx
from dotenv import load_dotenv

ROOT = Path(__file__).resolve().parents[1]
load_dotenv(ROOT / ".env")

APS_TOKEN_URL = "https://developer.api.autodesk.com/authentication/v2/token"
APS_AUTH_URL = "https://developer.api.autodesk.com/authentication/v2/authorize"
SCOPES = os.getenv("APS_SCOPES", "data:read data:write data:create")
REDIRECT_URI = os.getenv("APS_REDIRECT_URI", "http://localhost:8080/callback")
PORT = int(urllib.parse.urlparse(REDIRECT_URI).port or 8080)

result: dict[str, str] = {}


class CallbackHandler(BaseHTTPRequestHandler):
    def do_GET(self) -> None:  # noqa: N802
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path != urllib.parse.urlparse(REDIRECT_URI).path:
            self.send_response(404)
            self.end_headers()
            return
        params = urllib.parse.parse_qs(parsed.query)
        if "error" in params:
            result["error"] = params["error"][0]
            body = f"<h1>Auth failed</h1><p>{result['error']}</p>"
            self._respond(400, body)
            return
        if "code" not in params:
            result["error"] = "missing_code"
            self._respond(400, "<h1>Missing authorization code</h1>")
            return
        result["code"] = params["code"][0]
        self._respond(200, "<h1>Success</h1><p>You can close this window and return to the terminal.</p>")

    def _respond(self, status: int, body: str) -> None:
        self.send_response(status)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(body.encode())

    def log_message(self, format: str, *args) -> None:  # noqa: A003
        return


def main() -> int:
    client_id = os.getenv("APS_CLIENT_ID", "").strip()
    client_secret = os.getenv("APS_CLIENT_SECRET", "").strip()
    if not client_id or not client_secret:
        print("Set APS_CLIENT_ID and APS_CLIENT_SECRET in fusion-cloud/.env", file=sys.stderr)
        return 1

    params = {
        "response_type": "code",
        "client_id": client_id,
        "redirect_uri": REDIRECT_URI,
        "scope": SCOPES,
        "state": "fusion-cloud-login",
    }
    auth_url = f"{APS_AUTH_URL}?{urllib.parse.urlencode(params)}"

    server = HTTPServer(("127.0.0.1", PORT), CallbackHandler)
    thread = threading.Thread(target=server.handle_request, daemon=True)
    thread.start()

    print(f"Opening browser for Autodesk login...\n{auth_url}\n")
    webbrowser.open(auth_url)

    deadline = time.time() + 180
    while time.time() < deadline and "code" not in result and "error" not in result:
        time.sleep(0.2)

    server.server_close()

    if "error" in result:
        print(f"OAuth error: {result['error']}", file=sys.stderr)
        return 1
    if "code" not in result:
        print("Timed out waiting for OAuth callback.", file=sys.stderr)
        return 1

    response = httpx.post(
        APS_TOKEN_URL,
        data={
            "grant_type": "authorization_code",
            "code": result["code"],
            "redirect_uri": REDIRECT_URI,
        },
        auth=(client_id, client_secret),
        timeout=30.0,
    )
    response.raise_for_status()
    data = response.json()
    refresh = data.get("refresh_token")
    if not refresh:
        print("No refresh_token in response. Check APS app type is Traditional Web App.", file=sys.stderr)
        return 1

    print("\n=== Add this to Cursor Cloud Agent secrets ===\n")
    print(f"APS_CLIENT_ID={client_id}")
    print(f"APS_CLIENT_SECRET={client_secret}")
    print(f"APS_REFRESH_TOKEN={refresh}")
    print("\nAlso add them to fusion-cloud/.env for local testing.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
