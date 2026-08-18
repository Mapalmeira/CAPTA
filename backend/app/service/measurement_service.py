from datetime import date
from pathlib import Path

from app.models import IngestionResponse, MeasurementBatch
from app.repository.csv_repository import csv_repository


class MeasurementService:
    def ingest(self, batch: MeasurementBatch) -> IngestionResponse:
        daily_files = csv_repository.append(batch.measurements)
        return IngestionResponse(
            stored=len(batch.measurements),
            daily_files=[f"{day}.csv" for day in daily_files],
        )

    def export_path(self, day: date) -> Path | None:
        return csv_repository.export_path(day)


measurement_service = MeasurementService()
