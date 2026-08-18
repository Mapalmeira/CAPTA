import csv
from collections import defaultdict
from datetime import date, datetime
from pathlib import Path
from threading import Lock

from app.models import Measurement


DATA_DIR = Path(__file__).resolve().parents[2] / "data"


class CsvRepository:
    def __init__(self) -> None:
        self.data_dir = DATA_DIR
        self.lock = Lock()
        self.data_dir.mkdir(parents=True, exist_ok=True)

    def append(self, measurements: list[Measurement]) -> list[str]:
        measurements_by_day: dict[date, list[Measurement]] = defaultdict(list)
        for measurement in measurements:
            day = datetime.fromtimestamp(measurement.timestamp).date()
            measurements_by_day[day].append(measurement)

        with self.lock:
            for day, daily_measurements in measurements_by_day.items():
                path = self._path(day)
                write_header = not path.exists() or path.stat().st_size == 0
                with path.open("a", newline="", encoding="utf-8") as csv_file:
                    writer = csv.writer(csv_file)
                    if write_header:
                        writer.writerow(["timestamp", "current_amps"])
                    for measurement in daily_measurements:
                        writer.writerow([measurement.timestamp, measurement.current_amps])

        return sorted(day.isoformat() for day in measurements_by_day)

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


csv_repository = CsvRepository()
