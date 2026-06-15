#!/usr/bin/env python3
"""Smoke test APS cloud connection using environment credentials."""

from __future__ import annotations

import asyncio
import os
import sys
from pathlib import Path

from dotenv import load_dotenv

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
load_dotenv(ROOT / ".env")

from mcp_server.aps_api import list_hubs, search_designs  # noqa: E402
from mcp_server.auth import ApsAuth, AuthRequiredError  # noqa: E402


async def main() -> int:
    try:
        auth = ApsAuth.from_env()
    except ValueError as exc:
        print(f"CONFIG: {exc}")
        return 1

    print("Auth status:", auth.status())
    try:
        token = await auth.get_access_token()
    except AuthRequiredError as exc:
        print(f"AUTH: {exc}")
        return 1

    hubs = await list_hubs(token)
    print(f"OK: {len(hubs)} hub(s)")
    for hub in hubs:
        print(f"  - {hub['name']} ({hub['id']}) [{hub.get('type', '')}]")

    hint = os.getenv("FUSION_PROJECT_HINT", "growbox")
    if hint:
        print(f"\nSearching for designs matching '{hint}'...")
        matches = await search_designs(token, hint, max_results=10)
        if not matches:
            print("  No matches (try a different FUSION_PROJECT_HINT)")
        for match in matches:
            print(
                f"  - {match['name']} @ {match['project_name']}/{match.get('folder_path', '')}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(asyncio.run(main()))
