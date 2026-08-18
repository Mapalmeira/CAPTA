const SECRET_TOKEN = "";

function doPost(e) {
  const token = e.parameter.token;
  if (!validateToken(token)) {
    return ContentService.createTextOutput("Access denied");
  }

  const mode = e.parameter.mode;
  if (mode === "complete-csv") return receiveCompleteCsvMode(e);
  if (mode === "append-lines") return appendLinesMode(e);
  if (mode === "verify") return verificationMode();

  return ContentService.createTextOutput("Invalid mode.");
}

// Shared token validation.
function validateToken(token) {
  return token === SECRET_TOKEN;
}
