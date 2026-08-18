import os
from hmac import compare_digest

from fastapi import HTTPException, status
from fastapi.security import HTTPBearer

bearer = HTTPBearer(auto_error=False)
AUTH_TOKEN = os.getenv("CAPTA_AUTH_TOKEN", "").strip()
if not AUTH_TOKEN:
    raise RuntimeError("CAPTA_AUTH_TOKEN must be set")


def require_token(credentials) -> None:
    if credentials is None or not compare_digest(credentials.credentials, AUTH_TOKEN):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid authentication token",
        )
