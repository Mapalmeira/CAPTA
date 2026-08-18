from datetime import date, datetime, time, timedelta

from app.models import (
    DailyConsumptionPoint,
    DailyConsumptionResponse,
    InstantaneousPowerResponse,
    Measurement,
    PowerPoint,
)
from app.repository.csv_repository import csv_repository


class VisualizationService:
    def instantaneous_power(
        self,
        day: date,
        start_hour: int,
        end_hour: int,
        voltage: float,
    ) -> InstantaneousPowerResponse:
        start_timestamp, end_timestamp = self._day_interval(day, start_hour, end_hour)
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
        )

    def daily_consumption(
        self,
        start_day: date,
        voltage: float,
        days: int,
    ) -> DailyConsumptionResponse:
        points: list[DailyConsumptionPoint] = []

        for offset in range(days):
            current_day = start_day + timedelta(days=offset)
            measurements = csv_repository.read(current_day)
            if not measurements:
                continue

            start_timestamp, end_timestamp = self._day_interval(current_day, 0, 24)
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

    def _day_interval(self, day: date, start_hour: int, end_hour: int) -> tuple[int, int]:
        start = datetime.combine(day, time()) + timedelta(hours=start_hour)
        end = datetime.combine(day, time()) + timedelta(hours=end_hour)
        return int(start.timestamp()), int(end.timestamp()) - 1

    def _watt_hours(self, points: list[PowerPoint]) -> float:
        watt_seconds = 0.0
        for current, following in zip(points, points[1:]):
            seconds = following.timestamp - current.timestamp
            watt_seconds += (current.power_watts + following.power_watts) / 2 * seconds
        return watt_seconds / 3600


visualization_service = VisualizationService()
