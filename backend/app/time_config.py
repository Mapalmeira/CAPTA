import os
from zoneinfo import ZoneInfo, ZoneInfoNotFoundError


TIME_ZONE_NAME = (
    os.getenv("CAPTA_TIMEZONE", "UTC").strip()
    or "UTC"
)

try:
    TIME_ZONE = ZoneInfo(TIME_ZONE_NAME)
except (ValueError, ZoneInfoNotFoundError) as error:
    raise RuntimeError(f"Unknown CAPTA_TIMEZONE: {TIME_ZONE_NAME}") from error
