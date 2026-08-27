from datetime import date, datetime, time, timedelta

from app.models import (
    DailyConsumptionPoint,
    DailyConsumptionResponse,
    InstantaneousPowerResponse,
    Measurement,
    PowerPoint,
)
from app.repository.csv_repository import csv_repository
from app.time_config import TIME_ZONE, TIME_ZONE_NAME


DAILY_CONSUMPTION_DAYS = 30


class VisualizationService:
    def instantaneous_power(
        self,
        day: date,
        voltage: float,
    ) -> InstantaneousPowerResponse:
        start_timestamp, end_timestamp = self._day_interval(day)
        measurements = self._select_interval(
            csv_repository.read(day), start_timestamp, end_timestamp
        )
        points = [
            PowerPoint(
                timestamp=measurement.timestamp,
                power_watts=measurement.current_amps * voltage,
            )
            for measurement in measurements
        ]
        return InstantaneousPowerResponse(
            points=points,
            total_wh=self._watt_hours(points),
            timezone=TIME_ZONE_NAME,
        )

    def daily_consumption(
        self,
        start_day: date,
        voltage: float,
    ) -> DailyConsumptionResponse:
        points: list[DailyConsumptionPoint] = []

        for offset in range(DAILY_CONSUMPTION_DAYS):
            current_day = start_day + timedelta(days=offset)
            measurements = csv_repository.read(current_day)
            if not measurements:
                continue

            start_timestamp, end_timestamp = self._day_interval(current_day)
            selected = self._select_interval(
                measurements, start_timestamp, end_timestamp
            )
            power_points = [
                PowerPoint(
                    timestamp=measurement.timestamp,
                    power_watts=measurement.current_amps * voltage,
                )
                for measurement in selected
            ]
            points.append(
                DailyConsumptionPoint(
                    day=current_day,
                    consumption_kwh=self._watt_hours(power_points) / 1000,
                )
            )

        if not points:
            raise ValueError("No measurements exist in the selected 30-day period")

        return DailyConsumptionResponse(
            points=points,
            total_kwh=sum(point.consumption_kwh for point in points),
        )

    def _select_interval(
        self,
        measurements: list[Measurement],
        start_timestamp: int,
        end_timestamp: int,
    ) -> list[Measurement]:
        unique = {measurement.timestamp: measurement for measurement in measurements}
        selected = [
            unique[timestamp]
            for timestamp in sorted(unique)
            if start_timestamp <= timestamp <= end_timestamp
        ]
        if not selected:
            raise ValueError("No measurements exist in the selected interval")

        if selected[0].timestamp > start_timestamp:
            selected.insert(
                0,
                Measurement(
                    timestamp=start_timestamp,
                    current_amps=selected[0].current_amps,
                ),
            )
        if selected[-1].timestamp < end_timestamp:
            selected.append(
                Measurement(
                    timestamp=end_timestamp,
                    current_amps=selected[-1].current_amps,
                )
            )
        return selected

    def _day_interval(self, day: date) -> tuple[int, int]:
        start = datetime.combine(day, time(), tzinfo=TIME_ZONE)
        end = datetime.combine(day + timedelta(days=1), time(), tzinfo=TIME_ZONE)
        return int(start.timestamp()), int(end.timestamp()) - 1

    def _watt_hours(self, points: list[PowerPoint]) -> float:
        watt_seconds = 0.0
        for current, following in zip(points, points[1:]):
            seconds = following.timestamp - current.timestamp
            watt_seconds += (current.power_watts + following.power_watts) / 2 * seconds
        return watt_seconds / 3600


visualization_service = VisualizationService()
