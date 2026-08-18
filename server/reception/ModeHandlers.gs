function appendLinesMode(e) {
  const dataString = sanitizeDataString(e.parameter.data);
  const data = parseDataRows(dataString);
  const initialTimestamp = data[0][0] * 1000;
  const date = new Date(initialTimestamp);

  const formattedDate = date.getFullYear() + '-' +
                        String(date.getMonth() + 1).padStart(2, '0') + '-' +
                        String(date.getDate()).padStart(2, '0');

  const spreadsheet = getOrCreateSpreadsheet(formattedDate);
  appendRows(spreadsheet, data);

  return ContentService.createTextOutput("Data saved!");
}

function verificationMode() {
  const currentDate = new Date().toISOString().substring(0, 10);
  const spreadsheet = getOrCreateSpreadsheet(currentDate);

  const succeeded = runVerification(spreadsheet);
  return ContentService.createTextOutput(succeeded ? "Data saved!" : "Verification failed.");
}

function receiveCompleteCsvMode(e) {
  if (!e.postData) {
    throw new Error("postData is undefined");
  }

  let fileName = e.parameter.name;
  if (!fileName) {
    throw new Error("The URL is missing the 'name' parameter");
  }
  fileName = sanitizeDataString(fileName);

  const csvContents = e.postData.contents;
  const folder = getOrCreateFolder("completeData");
  const blob = Utilities.newBlob(csvContents, "text/csv", fileName);
  folder.createFile(blob);

  return ContentService.createTextOutput("Data saved!");
}
