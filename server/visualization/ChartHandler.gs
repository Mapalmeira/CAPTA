class ChartHandler {
  static createPowerChart(powerSheet) {
    console.log("[ChartHandler] createPowerChart: starting...");

    const sheet = powerSheet.sheet;
    const lastRow = sheet.getLastRow();

    if (lastRow < 2) {
      console.warn("[ChartHandler] createPowerChart: not enough data to generate a chart.");
      return;
    }

    const startDate = powerSheet.getValue("A1");
    const endDate = powerSheet.getValue(`A${lastRow}`);
    const totalHours = Math.min(Math.ceil((endDate - startDate) / 3600000), 24);

    const height = 600;
    const minimumWidth = 1600;
    const maximumWidth = 4000;
    const widthPerHour = 150;
    const width = Math.min(maximumWidth, Math.max(minimumWidth, totalHours * widthPerHour));

    const chart = sheet.newChart()
      .setChartType(Charts.ChartType.LINE)
      .addRange(sheet.getRange(`A1:B${lastRow}`))
      .setPosition(5, 3, 10, 10)
      .setOption("title", "Power Chart (W)")
      .setOption("hAxis", {
        title: "Time",
        format: "HH:mm",
        slantedText: false,
        gridlines: { count: Math.max(totalHours, 2) },
        viewWindow: { min: startDate, max: endDate }
      })
      .setOption("vAxis", {
        title: "Power (W)",
        viewWindow: { min: 0 },
        gridlines: { count: Math.ceil(8000 / 500) }
      })
      .setOption("width", width)
      .setOption("height", height)
      .build();

    sheet.insertChart(chart);
    console.log("[ChartHandler] createPowerChart: chart inserted successfully.");
  }

  static createDailyConsumptionChart(consumptionSheet) {
    console.log("[ChartHandler] createDailyConsumptionChart: starting...");

    const sheet = consumptionSheet.sheet;
    const lastRow = sheet.getLastRow();

    if (lastRow < 2) {
      console.warn("[ChartHandler] createDailyConsumptionChart: not enough data to generate a chart.");
      return;
    }

    const height = 600;
    const width = 1200;

    const values = sheet.getRange(`B2:B${lastRow}`).getValues().flat();
    const maximumValue = Math.max(...values);
    const roundedMaximum = Math.ceil(maximumValue / 5) * 5;

    const chart = sheet.newChart()
      .setChartType(Charts.ChartType.COLUMN)
      .addRange(sheet.getRange(`A1:B${lastRow}`))
      .setPosition(5, 3, 10, 10)
      .setOption("title", "Daily Consumption (kWh)")
      .setOption("hAxis", {
        title: "Day",
        format: "dd/MM",
        slantedText: true
      })
      .setOption("vAxis", {
        title: "Consumption (kWh)",
        viewWindow: { min: 0, max: roundedMaximum },
        gridlines: { count: roundedMaximum / 5 + 1}
      })
      .setOption("width", width)
      .setOption("height", height)
      .build();

    sheet.insertChart(chart);
    console.log("[ChartHandler] createDailyConsumptionChart: column chart inserted successfully.");
  }
}
