import csv
from collections import defaultdict
from datetime import date, datetime
from pathlib import Path
from threading import Lock

from app.models import Measurement
from app.time_config import TIME_ZONE


DATA_DIR = Path(__file__).resolve().parents[2] / "data"


class CsvRepository:
    def __init__(self, data_dir: Path = DATA_DIR) -> None:
        self.data_dir = data_dir
        self.lock = Lock()
        self.data_dir.mkdir(parents=True, exist_ok=True)

    def append(self, measurements: list[Measurement]) -> tuple[int, list[str]]:
        measurements_by_day: dict[date, list[Measurement]] = defaultdict(list)
        for measurement in measurements:
            day = datetime.fromtimestamp(measurement.timestamp, TIME_ZONE).date()
            measurements_by_day[day].append(measurement)

        stored = 0
        updated_days: list[str] = []

        with self.lock:
            for day, daily_measurements in measurements_by_day.items():
                path = self._path(day)
                existing_timestamps = self._read_timestamps(path)
                new_measurements = []

                for measurement in daily_measurements:
                    if measurement.timestamp in existing_timestamps:
                        continue
                    existing_timestamps.add(measurement.timestamp)
                    new_measurements.append(measurement)

                if not new_measurements:
                    continue

                write_header = not path.exists() or path.stat().st_size == 0
                with path.open("a", newline="", encoding="utf-8") as csv_file:
                    writer = csv.writer(csv_file)
                    if write_header:
                        writer.writerow(["timestamp", "current_amps"])
                    for measurement in new_measurements:
                        writer.writerow([measurement.timestamp, measurement.current_amps])

                stored += len(new_measurements)
                updated_days.append(day.isoformat())

        return stored, sorted(updated_days)

    def read(self, day: date) -> list[Measurement]:
        path = self._path(day)
        if not path.exists():
            return []

        with path.open(newline="", encoding="utf-8") as csv_file:
            return [
                Measurement(
                    timestamp=int(row["timestamp"]),
                    current_amps=float(row["current_amps"]),
                )
                for row in csv.DictReader(csv_file)
            ]

    def export_path(self, day: date) -> Path | None:
        path = self._path(day)
        return path if path.exists() else None

    def _path(self, day: date) -> Path:
        return self.data_dir / f"{day.isoformat()}.csv"

    def _read_timestamps(self, path: Path) -> set[int]:
        if not path.exists() or path.stat().st_size == 0:
            return set()

        with path.open(newline="", encoding="utf-8") as csv_file:
            return {int(row["timestamp"]) for row in csv.DictReader(csv_file)}


csv_repository = CsvRepository()
