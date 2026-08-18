class Sheet {
  constructor(sheetName) {
    const spreadsheet = SpreadsheetApp.getActiveSpreadsheet();
    this.sheet = spreadsheet.getSheetByName(sheetName);

    if (!this.sheet) {
      this.sheet = spreadsheet.insertSheet(sheetName);
    }

    this.lastRow = this.sheet.getLastRow();
  }

  clear() {
    this.sheet.getCharts().forEach(chart => this.sheet.removeChart(chart));
    this.sheet.clear();
    this.lastRow = 0;
  }

  setColumnWidths(startColumn, width) {
    const maxColumn = this.sheet.getMaxColumns();
    for (let i = startColumn; i <= maxColumn; i++) {
      this.sheet.setColumnWidth(i, width);
    }
  }

  getValue(cell) {
    return this.sheet.getRange(cell).getValue();
  }

  getRange(cellOrRange) {
    return this.sheet.getRange(cellOrRange);
  }

  setCellData(cell, data) {
    const range = this.getRange(cell);
    range.setValue(data);
  }

  setRangeData(targetRange, data) {
    const range = this.getRange(targetRange);
    range.setValues(data);
  }
}
