from pathlib import Path

from fastapi import FastAPI
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

from app.controller.measurement_controller import router as measurement_router
from app.controller.visualization_controller import router as visualization_router


app = FastAPI(title="CAPTA Backend")
app.include_router(measurement_router)
app.include_router(visualization_router)

static_dir = Path(__file__).parent / "static"
app.mount("/static", StaticFiles(directory=static_dir), name="static")


@app.get("/", include_in_schema=False)
def frontend() -> FileResponse:
    return FileResponse(static_dir / "index.html")
