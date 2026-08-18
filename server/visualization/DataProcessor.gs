class DataProcessor {
  static filterAndDeduplicateData(data) {
    const dataMap = new Map();

    for (const row of data) {
      const timestamp = row[0];

      if (timestamp === "" || timestamp === null) {
        console.warn("[DataProcessor] filterAndDeduplicateData: ignoring an empty row.");
        continue;
      }
      if (timestamp === -1) {
        console.warn(`[DataProcessor] filterAndDeduplicateData: removing invalid timestamp (-1) -> ${row}`);
        continue;
      }
      if (!dataMap.has(timestamp)) {
        dataMap.set(timestamp, row);
      } else {
        console.warn(`[DataProcessor] filterAndDeduplicateData: removing duplicate -> ${row}`);
      }
    }

    const sortedData = Array.from(dataMap.values()).sort((a, b) => a[0] - b[0]);
    data.length = 0;
    data.push(...sortedData);

    console.log(`[DataProcessor] ${data.length} valid rows after cleaning and sorting.`);
  }

  static selectDataInterval(data, day, startHour, endHour) {
    console.log("[DataProcessor] selectDataInterval: starting...");

    if (data.length === 0) {
      console.error("[DataProcessor] ERROR - No data available!");
      throw new Error("No data is available for the selected period.");
    }

    // Calculate the interval timestamps.
    const startDate = new Date(day.getTime());
    startDate.setHours(startHour, 0, 1, 0);
    const endDate = new Date(day.getTime());
    endDate.setHours(endHour, 0, 0, 0);
    endDate.setTime(endDate.getTime() - 1);
    const startTimestamp = Math.floor(startDate.getTime() / 1000);
    const endTimestamp = Math.floor(endDate.getTime() / 1000);

    // Filter the requested interval.
    const filteredData = data
      .filter(([timestamp]) => timestamp >= startTimestamp && timestamp <= endTimestamp);

    if (filteredData.length === 0) {
      throw new Error("No data exists in the selected time interval.");
    }

    // Add boundary points.
    const initialValue = filteredData[0][1];
    const finalValue = filteredData[filteredData.length - 1][1];

    filteredData.unshift([startTimestamp, initialValue]);
    filteredData.push([endTimestamp, finalValue]);

    console.log("[DataProcessor] boundary points added.");

    // Update the original array in place.
    data.length = 0;
    data.push(...filteredData);
    console.log(`[DataProcessor] selectDataInterval: ${data.length} points ready for the chart.`);
  }

  static convertToPower(data, voltage) {
    const convertedData = data.map(([timestamp, value]) => [new Date(timestamp * 1000), value * voltage]);
    data.length = 0;
    data.push(...convertedData);
  }

  static calculateWattHours(data) {
    if (data.length < 2) {
      throw new Error("[DataProcessor] calculateWattHours: insufficient data to calculate consumption.");
    }

    console.log(`[DataProcessor] calculateWattHours: measurements used: ${data.length}`);

    const time = data.map(item => item[0].getTime() / 1000);
    const power = data.map(item => item[1]);

    console.log(`[DataProcessor] calculateWattHours: detected interval - first: ${time[0]}, last: ${time[time.length - 1]}`);

    const consumptionWs = Calculator.integral(time, power);
    console.log(`[DataProcessor] calculateWattHours: calculated consumption: ${consumptionWs.toFixed(2)} Ws`);

    const consumptionWh = consumptionWs / 3600;
    console.log(`[DataProcessor] calculateWattHours: final consumption: ${consumptionWh.toFixed(2)} Wh`);
    return consumptionWh;
  }
}
