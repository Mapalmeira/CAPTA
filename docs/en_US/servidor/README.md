# Data Reception, Storage, and Visualization

This directory contains the components responsible for **receiving, storing, and displaying the data** sent by the ESP32. All backend logic is implemented with Google Apps Script, using Google Drive and Google Sheets as the database and visualization platform.

---

## Overview

The system is divided into two main parts:

### Reception

Contains the code responsible for processing requests from the ESP32 and storing the data in Google Drive. The data is saved in daily spreadsheets inside the `dadosPart/` folder.

Data is sent through **HTTPS POST requests**:

- **Continuous Sending**: small samples are sent periodically for real-time visualization.

The script includes security token verification, input sanitization, and rejection of invalid requests. Installation involves creating a Web App in Google Apps Script.

### Visualization

Contains the scripts that must be inserted into a Google Sheets spreadsheet. The spreadsheet enables automatic visualization of data through graphs triggered by specific cells.

Available visualization modes:

- Instantaneous power over a day (with time filtering).
- Daily total consumption over the last 30 days.

---

## Security Recommendations

- Use a **Google account dedicated exclusively to the system**.
- Share folders and spreadsheets with your personal account only as a "viewer" or "editor."
- When creating projects in Google Apps Script, use an **incognito window** to avoid conflicts between multiple logged-in accounts.

---

## Expected Google Drive Structure

For the system to function correctly, all elements must be gathered in a **single folder on Google Drive** (the folder name does not matter):

- **`dadosPart/` Folder**
  Daily spreadsheets containing data sent via POST.

- **Reception Apps Script Project**
  Deployed as a Web App, responsible for processing requests and storing the data.

- **Visualization Spreadsheet**
  Contains the scripts for generating graphs from the stored data.

---

## Step-by-Step: Deploying the Web App

1. Access [Google Apps Script](https://script.google.com/) with a dedicated system account.
2. Create a new project.
3. Add each `.gs` file from the `servidor/recebimento/` folder:
   - Click `+` → `Script`.
   - Paste the contents of each file (`Entrada.gs`, etc.).
4. In the menu, go to `Deploy` → `New Deployment`.
5. Choose **"Web App"**.
6. Configure:
   - **Execute as**: your dedicated account.
   - **Who has access**: anyone, even anonymous users.
7. Copy the generated deployment link and use it in the ESP32 code.
8. Grant the requested permissions (Drive and Sheets access).

### Notes

- The `dadosPart/` folder must be manually created in the same directory as the script.
- The security token must be configured both in the App Script and on the ESP32.
- Requests without a token or with an incorrect token are automatically ignored.

---

## Step-by-Step: Setting Up the Visualization Spreadsheet

1. Create a new spreadsheet in Google Sheets.
2. In the `Extensions` menu → `Apps Script`, open the script editor.
3. For each file in `servidor/exibicao/`:
   - Click `+` → `Script`.
   - Paste the contents of `AbaPlanilha.gs`, `Calculadora.gs`, etc.
4. Manually run the `PotenciaInstantanea` or `ConsumoDiario` function to prompt Google to request permissions.
5. Set up triggers:
   - Go to `Triggers`.
   - Add an `onEdit` trigger for the `aoEditar()` function.

### Spreadsheet Structure

The main sheet should contain:

- `C2`: Electrical voltage (110V, 220V, etc.).
- `C3`: Target date (`Date()`).
- `C4`: Start hour (0-24).
- `C5`: End hour (0-24).
- `D7` and `D8`: Triggers for generating graphs.

A ready-to-use example can be found in the `servidor/exibicao/exemplo_planilha/` folder.

---

## Integration with Storage

The spreadsheet scripts automatically locate the stored data in Google Drive based on the date.

The `dadosPart/` folder must contain files in the standard format: `YYYY-MM-DD`. If the sending system has been correctly implemented, this will happen automatically.

Correct Drive structure is essential for proper graph generation.

---

## Important Notes

- Apps Script creation might fail if multiple Google accounts are logged into the browser. To avoid this, use an **incognito window** with only the system account logged in.
- The project uses the `WiFiClientSecure` library with `client.setInsecure()` on the ESP32 to facilitate communication with the Apps Script, which uses dynamic certificates that are hard to validate.

---

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)