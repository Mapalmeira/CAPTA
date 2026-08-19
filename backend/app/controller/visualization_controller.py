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
) -> InstantaneousPowerResponse:
    try:
        return visualization_service.instantaneous_power(day, voltage)
    except ValueError as error:
        raise HTTPException(status_code=404, detail=str(error)) from error


@router.get("/daily-consumption", response_model=DailyConsumptionResponse)
def daily_consumption(
    start_day: Annotated[date, Query(alias="start_date")],
    voltage: Annotated[float, Query(gt=0)],
) -> DailyConsumptionResponse:
    try:
        return visualization_service.daily_consumption(start_day, voltage)
    except ValueError as error:
        raise HTTPException(status_code=404, detail=str(error)) from error
