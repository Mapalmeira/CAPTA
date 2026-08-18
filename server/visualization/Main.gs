function onEdit(e) {
  const mainSheet = new MainSheet();
  const editedCell = e.range.getA1Notation();

  console.log("[Main] onEdit: edited cell:", editedCell);

  const actions = {
    "D7": instantaneousPower,
    "D8": dailyConsumption
  };

  if (actions[editedCell]) {
    console.log("[Main] onEdit: running action for", editedCell);
    mainSheet.getRange(editedCell).setValue("RUN");
    actions[editedCell]();
  }
}

function instantaneousPower() {
  const mainSheet = new MainSheet();
  const day = mainSheet.getDay();
  const startHour = mainSheet.getStartHour();
  const endHour = mainSheet.getEndHour();
  const voltage = mainSheet.getVoltage();

  if (endHour > 24 || startHour < 0) {
    throw new Error("The hour range must be between 0 and 24.");
  }

  console.log("[Main] instantaneousPower: loading data...");
  let data = Importer.importByDate(day);
  if (!data) {
    throw new Error("No data is available for the selected day.");
  }

  DataProcessor.filterAndDeduplicateData(data);
  DataProcessor.selectDataInterval(data, day, startHour, endHour);

  console.log("[Main] instantaneousPower: creating a new sheet...");
  const powerSheet = new Sheet("Instantaneous power");
  powerSheet.sheet.getRange("A:A").setNumberFormat("hh:mm");
  powerSheet.setColumnWidths(3, 300);
  powerSheet.clear();

  DataProcessor.convertToPower(data, voltage);
  const totalConsumption = DataProcessor.calculateWattHours(data);
  powerSheet.setCellData("E3", `${totalConsumption.toFixed(2)} Wh`);
  powerSheet.setRangeData(`A1:B${data.length}`, data);

  ChartHandler.createPowerChart(powerSheet);
  console.log("[Main] instantaneousPower: chart created successfully!");
}

function dailyConsumption() {
  const mainSheet = new MainSheet();
  const day = mainSheet.getDay();
  const voltage = mainSheet.getVoltage();

  console.log("[Main] dailyConsumption: loading data...");
  const data = Importer.importLast30Days(day);

  let totalConsumptionKWh = 0;
  const processedData = data.map(([currentDay, dayData]) => {
    DataProcessor.filterAndDeduplicateData(dayData);
    DataProcessor.selectDataInterval(dayData, currentDay, 0, 24);
    DataProcessor.convertToPower(dayData, voltage);
    const consumptionWh = DataProcessor.calculateWattHours(dayData);
    const consumptionKWh = consumptionWh / 1000;
    totalConsumptionKWh += consumptionKWh;
    return [currentDay, consumptionKWh];
  });

  console.log("[Main] dailyConsumption: creating a new sheet...");
  const consumptionSheet = new Sheet("Daily consumption");
  consumptionSheet.setColumnWidths(3, 300);
  consumptionSheet.clear();

  consumptionSheet.setCellData("E3", `${totalConsumptionKWh.toFixed(3)} kWh`);
  consumptionSheet.setRangeData(`A1:B${processedData.length}`, processedData);
  consumptionSheet.getRange(`B1:B${processedData.length}`).setNumberFormat("0.00");

  ChartHandler.createDailyConsumptionChart(consumptionSheet);
}
