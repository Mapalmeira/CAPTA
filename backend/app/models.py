from datetime import date

from pydantic import BaseModel, Field


class Measurement(BaseModel):
    timestamp: int = Field(gt=0)
    current_amps: float = Field(ge=0, allow_inf_nan=False)


class MeasurementBatch(BaseModel):
    measurements: list[Measurement] = Field(min_length=1, max_length=5000)


class IngestionResponse(BaseModel):
    """Confirmation returned after a measurement batch is stored."""

    stored: int = Field(description="Number of measurements stored")
    daily_files: list[str] = Field(description="Daily CSV files updated by the batch")


class PowerPoint(BaseModel):
    timestamp: int
    power_watts: float


class InstantaneousPowerResponse(BaseModel):
    points: list[PowerPoint]
    total_wh: float


class DailyConsumptionPoint(BaseModel):
    day: date
    consumption_kwh: float


class DailyConsumptionResponse(BaseModel):
    points: list[DailyConsumptionPoint]
    total_kwh: float
