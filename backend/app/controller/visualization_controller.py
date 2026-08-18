from datetime import date
from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Query

from app.auth import require_token
from app.models import DailyConsumptionResponse, InstantaneousPowerResponse
from app.service.visualization_service import visualization_service


router = APIRouter(
    prefix="/api/visualizations",
    tags=["visualizations"],
    dependencies=[Depends(require_token)],
)


@router.get("/instantaneous-power", response_model=InstantaneousPowerResponse)
def instantaneous_power(
    day: Annotated[date, Query(alias="date")],
    voltage: Annotated[float, Query(gt=0)],
    start_hour: Annotated[int, Query(ge=0, le=23)] = 0,
    end_hour: Annotated[int, Query(ge=1, le=24)] = 24,
) -> InstantaneousPowerResponse:
    if start_hour >= end_hour:
        raise HTTPException(status_code=422, detail="start_hour must be less than end_hour")
    try:
        return visualization_service.instantaneous_power(
            day, start_hour, end_hour, voltage
        )
    except ValueError as error:
        raise HTTPException(status_code=404, detail=str(error)) from error


@router.get("/daily-consumption", response_model=DailyConsumptionResponse)
def daily_consumption(
    start_day: Annotated[date, Query(alias="start_date")],
    voltage: Annotated[float, Query(gt=0)],
    days: Annotated[int, Query(ge=1, le=90)] = 30,
) -> DailyConsumptionResponse:
    try:
        return visualization_service.daily_consumption(start_day, voltage, days)
    except ValueError as error:
        raise HTTPException(status_code=404, detail=str(error)) from error
