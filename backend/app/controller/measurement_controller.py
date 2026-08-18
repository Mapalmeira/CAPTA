from datetime import date

from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.responses import FileResponse

from app.auth import require_token
from app.models import IngestionResponse, MeasurementBatch
from app.service.measurement_service import measurement_service


router = APIRouter(
    prefix="/api/measurements",
    tags=["measurements"],
    dependencies=[Depends(require_token)],
)


@router.post("", response_model=IngestionResponse, status_code=status.HTTP_201_CREATED)
def ingest_measurements(batch: MeasurementBatch) -> IngestionResponse:
    return measurement_service.ingest(batch)


@router.get("/{day}/export", response_class=FileResponse)
def export_daily_csv(day: date) -> FileResponse:
    """Stream the selected daily CSV file as a download."""
    path = measurement_service.export_path(day)
    if path is None:
        raise HTTPException(status_code=404, detail="No data exists for this day")
    return FileResponse(path, media_type="text/csv", filename=path.name)
