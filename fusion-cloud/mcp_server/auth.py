"""APS OAuth token management for cloud / headless MCP use."""

from __future__ import annotations

import os
import time
from dataclasses import dataclass

import httpx

APS_TOKEN_URL = "https://developer.api.autodesk.com/authentication/v2/token"
APS_AUTH_URL = "https://developer.api.autodesk.com/authentication/v2/authorize"
DEFAULT_SCOPES = "data:read data:write data:create"


@dataclass
class TokenStore:
    access_token: str | None = None
    refresh_token: str | None = None
    expires_at: float = 0.0

    @property
    def is_valid(self) -> bool:
        return bool(self.access_token) and time.time() < self.expires_at - 60


class ApsAuth:
    def __init__(
        self,
        client_id: str,
        client_secret: str,
        *,
        scopes: str = DEFAULT_SCOPES,
        refresh_token: str | None = None,
    ) -> None:
        self.client_id = client_id
        self.client_secret = client_secret
        self.scopes = scopes
        self.store = TokenStore(refresh_token=refresh_token)

    @classmethod
    def from_env(cls) -> ApsAuth:
        client_id = os.environ.get("APS_CLIENT_ID", "").strip()
        client_secret = os.environ.get("APS_CLIENT_SECRET", "").strip()
        refresh_token = os.environ.get("APS_REFRESH_TOKEN", "").strip() or None
        scopes = os.environ.get("APS_SCOPES", DEFAULT_SCOPES).strip()
        if not client_id or not client_secret:
            raise ValueError("APS_CLIENT_ID and APS_CLIENT_SECRET are required")
        return cls(client_id, client_secret, scopes=scopes, refresh_token=refresh_token)

    def build_auth_url(self, redirect_uri: str, state: str = "fusion-cloud") -> str:
        from urllib.parse import urlencode

        params = {
            "response_type": "code",
            "client_id": self.client_id,
            "redirect_uri": redirect_uri,
            "scope": self.scopes,
            "state": state,
        }
        return f"{APS_AUTH_URL}?{urlencode(params)}"

    async def exchange_code(self, code: str, redirect_uri: str) -> dict:
        async with httpx.AsyncClient(timeout=30.0) as client:
            response = await client.post(
                APS_TOKEN_URL,
                data={
                    "grant_type": "authorization_code",
                    "code": code,
                    "redirect_uri": redirect_uri,
                },
                auth=(self.client_id, self.client_secret),
            )
            response.raise_for_status()
            return response.json()

    async def get_access_token(self) -> str:
        if self.store.is_valid:
            return self.store.access_token  # type: ignore[return-value]
        if not self.store.refresh_token:
            raise AuthRequiredError(self.missing_credentials_message())
        async with httpx.AsyncClient(timeout=30.0) as client:
            response = await client.post(
                APS_TOKEN_URL,
                data={
                    "grant_type": "refresh_token",
                    "refresh_token": self.store.refresh_token,
                    "scope": self.scopes,
                },
                auth=(self.client_id, self.client_secret),
            )
            response.raise_for_status()
            data = response.json()
        self.store = TokenStore(
            access_token=data["access_token"],
            refresh_token=data.get("refresh_token", self.store.refresh_token),
            expires_at=time.time() + data.get("expires_in", 3600),
        )
        return self.store.access_token  # type: ignore[return-value]

    def apply_token_response(self, data: dict) -> None:
        self.store = TokenStore(
            access_token=data["access_token"],
            refresh_token=data.get("refresh_token", self.store.refresh_token),
            expires_at=time.time() + data.get("expires_in", 3600),
        )

    def status(self) -> dict:
        return {
            "has_client_id": bool(self.client_id),
            "has_client_secret": bool(self.client_secret),
            "has_refresh_token": bool(self.store.refresh_token),
            "access_token_valid": self.store.is_valid,
            "scopes": self.scopes,
        }

    @staticmethod
    def missing_credentials_message() -> str:
        return (
            "Autodesk authentication required. Set APS_CLIENT_ID, APS_CLIENT_SECRET, and "
            "APS_REFRESH_TOKEN in your environment. Run `uv run python scripts/oauth_login.py` "
            "on your local machine once to obtain a refresh token, then add it as a Cursor secret."
        )


class AuthRequiredError(RuntimeError):
    pass
