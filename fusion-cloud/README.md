# Fusion Cloud MCP (APS)

Cloud-first access to your **Autodesk Fusion Team** projects via the [Autodesk Platform Services (APS) Data Management API](https://aps.autodesk.com/data-management-api). This lets a Cursor Cloud Agent browse your Fusion cloud files without Fusion desktop installed.

This is **not** the AuraFriday or Autodesk Fusion desktop MCP. It talks to Autodesk's cloud APIs directly.

## What this can do

| Capability | Supported |
|------------|-----------|
| List hubs and projects | Yes |
| Browse folders and files | Yes |
| Search designs by name (e.g. growbox, pcb) | Yes |
| Read file version metadata | Yes |
| Edit PCB geometry / schematics in cloud | **No** (needs Fusion Automation API or local Fusion MCP) |
| Download binary `.f3d` content | Not yet |

If cloud browse works but you need live edits, fall back to **local Fusion + AuraFriday MCP** or **Autodesk Fusion MCP** (desktop add-in).

## Prerequisites

1. **Autodesk Fusion subscription** with projects in Fusion Team cloud
2. **APS application** — [Create one](https://aps.autodesk.com/myapps) as **Traditional Web App**
3. **Callback URL** on the APS app: `http://localhost:8080/callback`
4. **API access**: enable Data Management API for the app

### Required OAuth scopes

```
data:read data:write data:create
```

## Setup (one-time, on your PC)

### 1. Create APS credentials

1. Go to https://aps.autodesk.com/myapps → **Create application**
2. Type: **Traditional Web App**
3. Callback URL: `http://localhost:8080/callback`
4. Copy **Client ID** and **Client Secret**

### 2. Get a refresh token

On your local machine:

```bash
cd fusion-cloud
cp .env.example .env
# Edit .env — set APS_CLIENT_ID and APS_CLIENT_SECRET

curl -LsSf https://astral.sh/uv/install.sh | sh   # if needed
uv sync
uv run python scripts/oauth_login.py
```

Sign in with your Autodesk account when the browser opens. The script prints three values.

### 3. Add secrets to Cursor Cloud Agent

In your Cursor Cloud Agent / repository secrets, add:

| Secret | Value |
|--------|-------|
| `APS_CLIENT_ID` | From APS app |
| `APS_CLIENT_SECRET` | From APS app |
| `APS_REFRESH_TOKEN` | From `oauth_login.py` output |

Optional: `FUSION_PROJECT_HINT=growbox` to tune the connection test search.

### 4. MCP configuration

The project includes `.cursor/mcp.json` pointing at this server. After secrets are set, restart the cloud agent so MCP loads.

## Verify connection

```bash
cd fusion-cloud
uv sync
uv run python scripts/test_connection.py
```

Expected without secrets: `AUTH: Autodesk authentication required...`  
Expected with secrets: hub list + design search results.

## MCP tools exposed

| Tool | Purpose |
|------|---------|
| `auth_status` | Check credentials and connection |
| `list_fusion_hubs` | List Fusion Team hubs |
| `list_fusion_projects` | Projects in a hub |
| `list_project_root_folders` | Top folders in a project |
| `list_fusion_folder_contents` | Files and subfolders |
| `get_fusion_item_versions` | Version history for a file |
| `search_fusion_designs` | Find files by name across all projects |

## Official Autodesk cloud MCP (alternative)

Autodesk also ships **Fusion Data MCP** (remote, no desktop Fusion) as part of their managed MCP offering. See:

- https://www.autodesk.com/solutions/autodesk-ai/autodesk-mcp-servers

That may offer richer data access once enabled on your subscription. This repo's server is a self-hosted APS alternative you control.

## Cloud edit path (future)

To **edit** designs in the cloud (not just browse), you would need:

- **Fusion Automation API** (APS premium, TypeScript scripts, pay-per-use)
- Or **Autodesk Fusion MCP** (local Fusion desktop)

We can add Automation API support in a follow-up if browse-only works but edit is required.

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `auth_required` | Run `oauth_login.py`, set `APS_REFRESH_TOKEN` secret |
| Empty hub list | Sign in with the Autodesk account that owns the Fusion projects |
| `403` on projects | Ensure Fusion Team (not only personal hub) and correct account |
| Search finds nothing | Try `search_fusion_designs` with part of your PCB filename |
| MCP tools not visible | Restart agent after adding secrets; check `.cursor/mcp.json` |
