# CAPTA Backend

FastAPI backend for receiving measurements, storing one CSV file per day, exporting those files, and serving the plain HTML/JavaScript dashboard.

## API

Send structured measurements with `POST /api/measurements`:

```json
{
  "measurements": [
    {"timestamp": 1776556800, "current_amps": 2.45},
    {"timestamp": 1776556860, "current_amps": 2.51}
  ]
}
```

Authenticated endpoints expect this header:

```text
Authorization: Bearer <token>
```

Other endpoints:

- `GET /api/measurements/{date}/export`
- `GET /api/visualizations/instantaneous-power`
- `GET /api/visualizations/daily-consumption`
- `GET /` for the dashboard

Open `/docs` for the generated API documentation.

## Run locally

Set the required environment variable and start Uvicorn from this directory:

```bash
export CAPTA_AUTH_TOKEN="replace-with-your-token"
uvicorn app.main:app --reload
```

Data is written to `backend/data/`. Daily files follow the operating system's timezone.

## Podman

Build the image:

```bash
podman build -t capta-backend -f backend/Containerfile backend
```

Create the secret from a local file:

```bash
podman secret create capta_auth_token ./token.txt
```

Run the container and expose the secret as the required environment variable:

```bash
podman run --rm \
  --name capta-backend \
  -p 8000:8000 \
  --secret capta_auth_token,type=env,target=CAPTA_AUTH_TOKEN \
  -v capta-data:/app/data \
  capta-backend
```
