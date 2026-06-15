"""APS Data Management API helpers for Fusion Team cloud access."""

from __future__ import annotations

from typing import Any

import httpx

APS_BASE_URL = "https://developer.api.autodesk.com"


def _auth_headers(access_token: str) -> dict[str, str]:
    return {"Authorization": f"Bearer {access_token}"}


async def list_hubs(access_token: str) -> list[dict[str, Any]]:
    async with httpx.AsyncClient(timeout=30.0) as client:
        response = await client.get(
            f"{APS_BASE_URL}/project/v1/hubs",
            headers=_auth_headers(access_token),
        )
        response.raise_for_status()
        data = response.json()
    return [
        {
            "id": hub["id"],
            "name": hub["attributes"]["name"],
            "type": hub["attributes"].get("extension", {}).get("type", ""),
            "region": hub["attributes"].get("region", ""),
        }
        for hub in data.get("data", [])
    ]


async def list_projects(access_token: str, hub_id: str) -> list[dict[str, Any]]:
    async with httpx.AsyncClient(timeout=30.0) as client:
        response = await client.get(
            f"{APS_BASE_URL}/project/v1/hubs/{hub_id}/projects",
            headers=_auth_headers(access_token),
        )
        response.raise_for_status()
        data = response.json()
    return [
        {
            "id": project["id"],
            "name": project["attributes"]["name"],
            "project_type": project["attributes"].get("extension", {}).get("data", {}).get("projectType", ""),
        }
        for project in data.get("data", [])
    ]


async def list_top_folders(
    access_token: str, hub_id: str, project_id: str
) -> list[dict[str, Any]]:
    async with httpx.AsyncClient(timeout=30.0) as client:
        response = await client.get(
            f"{APS_BASE_URL}/project/v1/hubs/{hub_id}/projects/{project_id}/topFolders",
            headers=_auth_headers(access_token),
        )
        response.raise_for_status()
        data = response.json()
    return [_summarize_folder(entry) for entry in data.get("data", [])]


async def list_folder_contents(
    access_token: str, project_id: str, folder_id: str
) -> dict[str, Any]:
    async with httpx.AsyncClient(timeout=30.0) as client:
        response = await client.get(
            f"{APS_BASE_URL}/data/v1/projects/{project_id}/folders/{folder_id}/contents",
            headers=_auth_headers(access_token),
        )
        response.raise_for_status()
        data = response.json()
    folders = [_summarize_folder(entry) for entry in data.get("data", []) if entry["type"] == "folders"]
    items = [_summarize_item(entry) for entry in data.get("data", []) if entry["type"] == "items"]
    return {"folders": folders, "items": items}


async def list_item_versions(access_token: str, project_id: str, item_id: str) -> list[dict[str, Any]]:
    async with httpx.AsyncClient(timeout=30.0) as client:
        response = await client.get(
            f"{APS_BASE_URL}/data/v1/projects/{project_id}/items/{item_id}/versions",
            headers=_auth_headers(access_token),
        )
        response.raise_for_status()
        data = response.json()
    versions: list[dict[str, Any]] = []
    for version in data.get("data", []):
        attrs = version.get("attributes", {})
        ext = attrs.get("extension", {}).get("data", {})
        versions.append(
            {
                "id": version["id"],
                "name": attrs.get("name", ""),
                "create_time": attrs.get("createTime", ""),
                "mime_type": ext.get("mimeType", ""),
                "file_type": ext.get("fileType", ""),
                "storage_size": ext.get("storageSize", 0),
            }
        )
    return versions


async def search_designs(
    access_token: str,
    name_contains: str,
    *,
    max_results: int = 25,
    max_folders_to_visit: int = 200,
) -> list[dict[str, Any]]:
    """Walk Fusion Team projects and return items whose name matches (case-insensitive)."""
    query = name_contains.lower()
    matches: list[dict[str, Any]] = []
    visited = 0

    hubs = await list_hubs(access_token)
    for hub in hubs:
        projects = await list_projects(access_token, hub["id"])
        for project in projects:
            if len(matches) >= max_results:
                return matches
            try:
                top_folders = await list_top_folders(access_token, hub["id"], project["id"])
            except httpx.HTTPStatusError:
                continue
            queue: list[tuple[dict[str, Any], str]] = [(folder, "") for folder in top_folders]
            while queue and len(matches) < max_results and visited < max_folders_to_visit:
                folder, parent_path = queue.pop(0)
                folder_name = folder.get("name", "")
                folder_path = f"{parent_path}/{folder_name}" if parent_path else folder_name
                visited += 1
                try:
                    contents = await list_folder_contents(access_token, project["id"], folder["id"])
                except httpx.HTTPStatusError:
                    continue
                for item in contents["items"]:
                    if query in item.get("name", "").lower():
                        matches.append(
                            {
                                **item,
                                "hub_id": hub["id"],
                                "hub_name": hub["name"],
                                "project_id": project["id"],
                                "project_name": project["name"],
                                "folder_path": folder_path,
                            }
                        )
                        if len(matches) >= max_results:
                            return matches
                for child in contents["folders"]:
                    queue.append((child, folder_path))
    return matches


def _summarize_folder(entry: dict[str, Any]) -> dict[str, Any]:
    attrs = entry.get("attributes", {})
    return {
        "id": entry["id"],
        "name": attrs.get("name", ""),
        "type": entry.get("type", "folders"),
    }


def _summarize_item(entry: dict[str, Any]) -> dict[str, Any]:
    attrs = entry.get("attributes", {})
    ext = attrs.get("extension", {}).get("data", {})
    return {
        "id": entry["id"],
        "name": attrs.get("name", ""),
        "type": entry.get("type", "items"),
        "file_type": ext.get("fileType", ""),
        "mime_type": ext.get("mimeType", ""),
    }
