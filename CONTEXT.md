# CONTEXT.md

## 2026-06-15 — Fusion cloud MCP setup (branch: cursor/fusion-cloud-mcp-setup-6549)

**Goal:** Cloud-first access to user's Autodesk Fusion Team PCB projects from Cursor Cloud Agent.

**What was added:**
- `fusion-cloud/` — APS Data Management MCP server (browse/search cloud Fusion files)
- `.cursor/mcp.json` — registers `fusion-cloud` MCP server
- `fusion-cloud/scripts/oauth_login.py` — one-time local OAuth to get refresh token
- `fusion-cloud/scripts/test_connection.py` — connection smoke test

**Blocked until user provides secrets:**
- `APS_CLIENT_ID`, `APS_CLIENT_SECRET`, `APS_REFRESH_TOKEN` (from APS app + oauth_login.py)

**Cloud vs local:**
- This path reads Fusion **cloud** metadata only; cannot edit PCB geometry
- If browse fails or edit needed → local Fusion + AuraFriday MCP or Autodesk Fusion desktop MCP

**Project firmware:** ESP32 growbox (DFRobot FireBeetle 2 ESP32-E); no PCB files in this git repo.

**Next after secrets:** Run `auth_status` / `search_fusion_designs` MCP tools; search for growbox/pcb filenames.
