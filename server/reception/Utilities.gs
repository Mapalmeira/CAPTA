function runVerification(spreadsheet) {
  try {
    const sheet = spreadsheet.getSheets()[0];
    sheet.getRange("Z1").setValue("test");
    sheet.getRange("Z1").clear();
    return true;
  } catch (e) {
    Logger.log("Verification error: " + e);
    return false;
  }
}

function parseDataRows(dataString) {
  if (typeof dataString !== "string" || dataString.trim() === "") {
    throw new Error("The input must be a non-empty string.");
  }

  const rawRows = dataString.split("_");
  const result = [];

  for (let i = 0; i < rawRows.length; i++) {
    const row = rawRows[i].trim();
    const parts = row.split("|");

    if (parts.length < 2) {
      throw new Error(`Invalid row ${i + 1}: it must contain at least a timestamp and one value.`);
    }

    const timestamp = parseInt(parts[0]);
    if (isNaN(timestamp)) {
      throw new Error(`Invalid row ${i + 1}: timestamp is not a valid number.`);
    }

    const values = parts.slice(1).map((value, valueIndex) => {
      const number = Number(value);
      if (isNaN(number)) {
        throw new Error(`Row ${i + 1}, value ${valueIndex + 1}: '${value}' is not a valid number.`);
      }
      return number;
    });

    result.push([timestamp, ...values]);
  }

  return result;
}

function getOrCreateSpreadsheet(dateString) {
  const fileName = dateString;
  const folder = getOrCreateFolder("partialData");

  const files = folder.getFilesByName(fileName);
  if (files.hasNext()) {
    return SpreadsheetApp.open(files.next());
  }

  const newSpreadsheet = SpreadsheetApp.create(fileName);
  const id = newSpreadsheet.getId();
  const file = DriveApp.getFileById(id);
  file.moveTo(folder);
  return newSpreadsheet;
}


function getOrCreateFolder(name) {
  const folders = DriveApp.getFoldersByName(name);
  if (folders.hasNext()) {
    return folders.next();
  }
  return DriveApp.createFolder(name);
}

function appendRows(spreadsheet, rows) {
  const sheet = spreadsheet.getSheets()[0];
  sheet.getRange(sheet.getLastRow() + 1, 1, rows.length, rows[0].length).setValues(rows);
}

function sanitizeDataString(str) {
  return str.replace(/[^a-zA-Z0-9._|,-]/g, '');
}
