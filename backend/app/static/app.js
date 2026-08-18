const today = new Date().toISOString().slice(0, 10);
for (const id of ["power-date", "consumption-date", "export-date"]) {
  document.getElementById(id).value = today;
}

function token() {
  return document.getElementById("token").value;
}

async function api(path) {
  document.getElementById("error").textContent = "";
  const response = await fetch(path, {
    headers: { Authorization: `Bearer ${token()}` },
  });
  if (!response.ok) {
    const body = await response.json().catch(() => ({}));
    throw new Error(body.detail || `Request failed (${response.status})`);
  }
  return response;
}

function drawChart(canvasId, values, labels, color) {
  const canvas = document.getElementById(canvasId);
  const ratio = window.devicePixelRatio || 1;
  const width = canvas.clientWidth;
  const height = 280;
  canvas.width = width * ratio;
  canvas.height = height * ratio;

  const context = canvas.getContext("2d");
  context.scale(ratio, ratio);
  context.clearRect(0, 0, width, height);
  if (!values.length) return;

  const padding = 42;
  const maximum = Math.max(...values, 1);
  const chartWidth = width - padding * 2;
  const chartHeight = height - padding * 2;

  context.strokeStyle = "#c8d0d8";
  context.beginPath();
  context.moveTo(padding, padding);
  context.lineTo(padding, height - padding);
  context.lineTo(width - padding, height - padding);
  context.stroke();

  context.strokeStyle = color;
  context.lineWidth = 2;
  context.beginPath();
  values.forEach((value, index) => {
    const x = padding + chartWidth * (values.length === 1 ? 0 : index / (values.length - 1));
    const y = height - padding - chartHeight * (value / maximum);
    if (index === 0) context.moveTo(x, y);
    else context.lineTo(x, y);
  });
  context.stroke();

  context.fillStyle = "#53606d";
  context.font = "12px system-ui";
  context.fillText("0", 16, height - padding + 4);
  context.fillText(maximum.toFixed(2), 4, padding + 4);
  context.fillText(labels[0] || "", padding, height - 12);
  context.textAlign = "right";
  context.fillText(labels.at(-1) || "", width - padding, height - 12);
  context.textAlign = "left";
}

document.getElementById("power-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const query = new URLSearchParams({
      date: document.getElementById("power-date").value,
      start_hour: document.getElementById("start-hour").value,
      end_hour: document.getElementById("end-hour").value,
      voltage: document.getElementById("power-voltage").value,
    });
    const data = await (await api(`/api/visualizations/instantaneous-power?${query}`)).json();
    document.getElementById("power-summary").textContent = `${data.total_wh.toFixed(2)} Wh in the selected interval`;
    drawChart(
      "power-chart",
      data.points.map((point) => point.power_watts),
      data.points.map((point) => new Date(point.timestamp * 1000).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" })),
      "#176b55",
    );
  } catch (error) {
    document.getElementById("error").textContent = error.message;
  }
});

document.getElementById("consumption-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const query = new URLSearchParams({
      start_date: document.getElementById("consumption-date").value,
      voltage: document.getElementById("consumption-voltage").value,
      days: document.getElementById("days").value,
    });
    const data = await (await api(`/api/visualizations/daily-consumption?${query}`)).json();
    document.getElementById("consumption-summary").textContent = `${data.total_kwh.toFixed(3)} kWh total`;
    drawChart(
      "consumption-chart",
      data.points.map((point) => point.consumption_kwh),
      data.points.map((point) => point.day.slice(5)),
      "#315b9d",
    );
  } catch (error) {
    document.getElementById("error").textContent = error.message;
  }
});

document.getElementById("export-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const day = document.getElementById("export-date").value;
    const blob = await (await api(`/api/measurements/${day}/export`)).blob();
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = `${day}.csv`;
    link.click();
    URL.revokeObjectURL(link.href);
  } catch (error) {
    document.getElementById("error").textContent = error.message;
  }
});
