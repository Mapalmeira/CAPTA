class Importer {
  static importByDate(date) {
    const baseName = Utilities.formatDate(date, Session.getScriptTimeZone(), "yyyy-MM-dd");

    // Try the completeData/ folder first.
    const csvFolder = Importer._getFolderByName("completeData");
    const csvFile = Importer._getFileByName(csvFolder, `${baseName}.csv`);

    if (csvFile) {
      console.log(`[Importer] Importing CSV: ${baseName}.csv`);
      return Importer._readCsvAsArray(csvFile);
    }

    // If no CSV exists, try the partialData/ folder.
    const spreadsheetFolder = Importer._getFolderByName("partialData");
    const spreadsheetFile = Importer._getFileByName(spreadsheetFolder, baseName);

    if (spreadsheetFile) {
      console.log(`[Importer] Importing spreadsheet: ${baseName}`);
      return Importer._readSpreadsheetAsArray(spreadsheetFile);
    }

    // No data source was found.
    return null;
  }

  static importLast30Days(day) {
    const days = [];

    for (let i = 30; i >= 0; i--) {
      const currentDate = new Date(day);
      currentDate.setDate(day.getDate() - i);

      const data = Importer.importByDate(currentDate);
      if (data !== null) {
        days.push([currentDate, data]);
      }
    }

    return days;
  }

  // Helper methods

  static _getFolderByName(name) {
    const folders = DriveApp.getFoldersByName(name);
    if (!folders.hasNext()) {
      throw new Error(`[Importer] Folder "${name}" was not found.`);
    }
    return folders.next();
  }

  static _getFileByName(folder, fileName) {
    const files = folder.getFilesByName(fileName);
    return files.hasNext() ? files.next() : null;
  }

  static _readCsvAsArray(file) {
    const contents = file.getBlob().getDataAsString();

    if (typeof contents !== "string" || contents.trim() === "") {
      throw new Error("[Importer] The CSV file is empty or invalid.");
    }

    const rows = Utilities.parseCsv(contents);

    if (!Array.isArray(rows)) {
      throw new Error("[Importer] The parsed CSV content is not a row list.");
    }

    const validRows = [];
    for (let i = 0; i < rows.length; i++) {
      const row = rows[i];

      if (!Array.isArray(row) || row.length < 2) {
        console.warn(`[Importer] Row ${i + 1} ignored: invalid format.`);
        continue;
      }

      const timestamp = parseInt(row[0], 10);
      const value = Number(row[1]);

      if (isNaN(timestamp) || isNaN(value)) {
        console.warn(`[Importer] Row ${i + 1} ignored: timestamp or value is not numeric.`);
        continue;
      }

      validRows.push([timestamp, value]);
    }

    return validRows;
  }

  static _readSpreadsheetAsArray(file) {
    const spreadsheet = SpreadsheetApp.open(file);
    const sheet = spreadsheet.getSheets()[0];
    const data = sheet.getDataRange().getValues();

    if (!Array.isArray(data)) {
      throw new Error("[Importer] Reading the spreadsheet did not return a data matrix.");
    }

    const validRows = [];
    for (let i = 0; i < data.length; i++) {
      const row = data[i];

      if (!Array.isArray(row) || row.length < 2) {
        console.warn(`[Importer] Row ${i + 1} ignored: fewer than two columns.`);
        continue;
      }

      const timestamp = parseInt(row[0], 10);
      const value = Number(row[1]);

      if (isNaN(timestamp) || isNaN(value)) {
        console.warn(`[Importer] Row ${i + 1} ignored: timestamp or value is invalid.`);
        continue;
      }

      validRows.push([timestamp, value]);
    }

    return validRows;
  }
}
