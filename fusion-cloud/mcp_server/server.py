"""
Cloud MCP server for Autodesk Fusion Team (APS Data Management API).

Reads your Fusion cloud projects without local Fusion desktop. Uses a refresh
token obtained via scripts/oauth_login.py for headless cloud agent use.

Cloud editing of geometry is NOT supported here — that requires either:
  - Autodesk Fusion MCP (local Fusion desktop), or
  - Fusion Automation API (separate APS premium API, TypeScript scripts).

This server supports: browse hubs/projects/folders, find designs, read metadata.
"""

from __future__ import annotations

import os

from dotenv import load_dotenv
from fastmcp import FastMCP

from mcp_server.aps_api import (
    list_folder_contents,
    list_hubs,
    list_item_versions,
    list_projects,
    list_top_folders,
    search_designs,
)
from mcp_server.auth import ApsAuth, AuthRequiredError

load_dotenv()

mcp = FastMCP(
    "Fusion Cloud (APS)",
    instructions=(
        "Access Autodesk Fusion Team cloud files via APS Data Management API. "
        "Use list_hubs → list_projects → list_top_folders → list_folder_contents "
        "to navigate. Use search_designs to find PCB/Fusion files by name. "
        "Requires APS_REFRESH_TOKEN in environment (see fusion-cloud/README.md)."
    ),
)

_auth: ApsAuth | None = None


def _get_auth() -> ApsAuth:
    global _auth
    if _auth is None:
        _auth = ApsAuth.from_env()
    return _auth


async def _token() -> str:
    return await _get_auth().get_access_token()


def _auth_error_payload(exc: AuthRequiredError) -> dict:
    auth = _get_auth()
    redirect = os.environ.get("APS_REDIRECT_URI", "http://localhost:8080/callback")
    return {
        "error": "auth_required",
        "message": str(exc),
        "auth_status": auth.status(),
        "setup_url_hint": auth.build_auth_url(redirect),
        "setup_instructions": (
            "Run fusion-cloud/scripts/oauth_login.py on your PC, then set "
            "APS_REFRESH_TOKEN in Cursor Cloud Agent secrets and restart the agent."
        ),
    }


@mcp.tool()
async def auth_status() -> dict:
    """Check whether APS credentials and refresh token are configured."""
    try:
        auth = _get_auth()
        status = auth.status()
        if status["has_refresh_token"]:
            try:
                await auth.get_access_token()
                status["connection"] = "ok"
            except Exception as exc:  # noqa: BLE001
                status["connection"] = "failed"
                status["connection_error"] = str(exc)
        else:
            status["connection"] = "not_configured"
        return status
    except ValueError as exc:
        return {"connection": "not_configured", "error": str(exc)}


@mcp.tool()
async def list_fusion_hubs() -> list[dict] | dict:
    """List Fusion Team / BIM hubs accessible to your Autodesk account."""
    try:
        token = await _token()
        return await list_hubs(token)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


@mcp.tool()
async def list_fusion_projects(hub_id: str) -> list[dict] | dict:
    """List projects in a Fusion hub. Use list_fusion_hubs first to get hub_id."""
    try:
        token = await _token()
        return await list_projects(token, hub_id)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


@mcp.tool()
async def list_project_root_folders(hub_id: str, project_id: str) -> list[dict] | dict:
    """List top-level folders in a Fusion project."""
    try:
        token = await _token()
        return await list_top_folders(token, hub_id, project_id)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


@mcp.tool()
async def list_fusion_folder_contents(project_id: str, folder_id: str) -> dict:
    """List subfolders and design files inside a folder."""
    try:
        token = await _token()
        return await list_folder_contents(token, project_id, folder_id)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


@mcp.tool()
async def get_fusion_item_versions(project_id: str, item_id: str) -> list[dict] | dict:
    """List versions of a Fusion design file (items from list_fusion_folder_contents)."""
    try:
        token = await _token()
        return await list_item_versions(token, project_id, item_id)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


@mcp.tool()
async def search_fusion_designs(
    name_contains: str,
    max_results: int = 25,
) -> list[dict] | dict:
    """
    Search all accessible Fusion projects for files whose name contains the query.
    Example queries: growbox, pcb, fission, esp32, canna
    """
    try:
        token = await _token()
        return await search_designs(token, name_contains, max_results=max_results)
    except AuthRequiredError as exc:
        return _auth_error_payload(exc)


def main() -> None:
    mcp.run()


if __name__ == "__main__":
    main()
