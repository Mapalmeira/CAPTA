function dateInputValue(date) {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, "0");
  const day = String(date.getDate()).padStart(2, "0");
  return `${year}-${month}-${day}`;
}

const today = dateInputValue(new Date());
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

const chartStates = new Map();

function niceScale(maximum, targetTicks = 5) {
  const safeMaximum = maximum > 0 ? maximum : 1;
  const roughStep = safeMaximum / targetTicks;
  const magnitude = 10 ** Math.floor(Math.log10(roughStep));
  const normalized = roughStep / magnitude;
  let niceNormalized;

  if (normalized <= 1) niceNormalized = 1;
  else if (normalized <= 2) niceNormalized = 2;
  else if (normalized <= 2.5) niceNormalized = 2.5;
  else if (normalized <= 5) niceNormalized = 5;
  else niceNormalized = 10;

  const step = niceNormalized * magnitude;
  return { maximum: Math.ceil(safeMaximum / step) * step, step };
}

function axisNumber(value, step) {
  if (step >= 10) return value.toFixed(0);
  if (step >= 1) return value.toFixed(1).replace(/\.0$/, "");
  return value.toFixed(2).replace(/0+$/, "").replace(/\.$/, "");
}

function xTickIndexes(pointCount, maximumTicks) {
  if (pointCount <= maximumTicks) return Array.from({ length: pointCount }, (_, index) => index);

  const indexes = new Set();
  for (let tick = 0; tick < maximumTicks; tick += 1) {
    indexes.add(Math.round((tick * (pointCount - 1)) / (maximumTicks - 1)));
  }
  return [...indexes];
}

function hourLabel(timestamp, timezone) {
  return new Intl.DateTimeFormat("pt-BR", {
    hour: "2-digit",
    minute: "2-digit",
    hourCycle: "h23",
    timeZone: timezone,
  }).format(new Date(timestamp * 1000));
}

function dayLabel(day) {
  const [, month, date] = day.split("-");
  return `${date}/${month}`;
}

function chartBaseWidth(state) {
  const viewportWidth = Math.max(state.scrollContainer.clientWidth, 280);
  const mobileWidth = Number(state.canvas.dataset.mobileWidth) || viewportWidth;
  return window.matchMedia("(max-width: 640px)").matches
    ? Math.max(viewportWidth, mobileWidth)
    : viewportWidth;
}

function updateZoomControls(state) {
  if (!state.toolbar) return;
  state.toolbar.querySelector(".zoom-value").value = `${Math.round(state.zoom * 100)}%`;
  state.toolbar.querySelector('[data-zoom="out"]').disabled = state.zoom <= 1;
  state.toolbar.querySelector('[data-zoom="in"]').disabled = state.zoom >= 4;
  state.toolbar.querySelector('[data-zoom="reset"]').disabled = state.zoom === 1;
}

function resizeChart(state) {
  state.canvas.style.width = `${Math.round(chartBaseWidth(state) * state.zoom)}px`;
  updateZoomControls(state);
  renderChart(state);
}

function setChartZoom(state, nextZoom, focusX = state.scrollContainer.clientWidth / 2) {
  const zoom = Math.min(4, Math.max(1, nextZoom));
  if (zoom === state.zoom) return;

  const oldWidth = state.canvas.clientWidth || chartBaseWidth(state) * state.zoom;
  const contentFocus = state.scrollContainer.scrollLeft + focusX;
  state.zoom = zoom;
  state.canvas.style.width = `${Math.round(chartBaseWidth(state) * state.zoom)}px`;
  const widthRatio = state.canvas.clientWidth / oldWidth;
  state.scrollContainer.scrollLeft = contentFocus * widthRatio - focusX;
  updateZoomControls(state);
  renderChart(state);
}

function renderChart(state) {
  const { canvas, points, options } = state;
  const context = canvas.getContext("2d");
  const ratio = window.devicePixelRatio || 1;
  const width = Math.max(canvas.clientWidth, 280);
  const height = 340;
  const margin = { top: 28, right: 22, bottom: 48, left: 66 };
  const chartWidth = width - margin.left - margin.right;
  const chartHeight = height - margin.top - margin.bottom;

  canvas.width = Math.round(width * ratio);
  canvas.height = Math.round(height * ratio);
  context.setTransform(ratio, 0, 0, ratio, 0, 0);
  context.clearRect(0, 0, width, height);
  if (!points.length) return;

  const scale = niceScale(Math.max(...points.map((point) => point.value)));
  const regularXInterval = options.xTickInterval;
  const rawMinimumX = Math.min(...points.map((point) => point.x));
  const rawMaximumX = Math.max(...points.map((point) => point.x));
  let minimumX = regularXInterval
    ? Math.floor(rawMinimumX / regularXInterval) * regularXInterval
    : rawMinimumX;
  let maximumX = regularXInterval
    ? Math.ceil(rawMaximumX / regularXInterval) * regularXInterval
    : rawMaximumX;
  if (regularXInterval && minimumX === maximumX) maximumX += regularXInterval;
  const xPosition = (value, index) => {
    if (minimumX === maximumX) return margin.left + chartWidth / 2;
    if (!Number.isFinite(value)) return margin.left + (chartWidth * index) / Math.max(points.length - 1, 1);
    return margin.left + chartWidth * ((value - minimumX) / (maximumX - minimumX));
  };
  const yPosition = (value) => margin.top + chartHeight * (1 - value / scale.maximum);

  context.font = "12px system-ui";
  context.lineWidth = 1;
  context.textBaseline = "middle";
  const yTicks = Math.round(scale.maximum / scale.step);
  for (let tick = 0; tick <= yTicks; tick += 1) {
    const value = tick * scale.step;
    const y = yPosition(value);
    context.strokeStyle = tick === 0 ? "#aeb8c2" : "#e4e8ec";
    context.beginPath();
    context.moveTo(margin.left, y);
    context.lineTo(width - margin.right, y);
    context.stroke();
    context.fillStyle = "#53606d";
    context.textAlign = "right";
    context.fillText(axisNumber(value, scale.step), margin.left - 10, y);
  }

  context.fillStyle = "#53606d";
  context.textAlign = "left";
  context.textBaseline = "alphabetic";
  context.fillText(options.unit, margin.left, 16);

  const xTicks = [];
  if (regularXInterval) {
    for (let value = minimumX; value <= maximumX; value += regularXInterval) {
      xTicks.push({ value, label: options.xTickFormatter(value) });
    }
  } else {
    points.forEach((point, index) => xTicks.push({ value: point.x, label: point.label, pointIndex: index }));
  }

  const maximumXLabels = Math.max(2, Math.min(8, Math.floor(chartWidth / 72)));
  const labelIndexes = new Set(xTickIndexes(xTicks.length, maximumXLabels));
  context.textBaseline = "top";
  xTicks.forEach((tick, tickIndex) => {
    const x = xPosition(tick.value, tick.pointIndex ?? tickIndex);
    context.strokeStyle = "#eef1f4";
    context.beginPath();
    context.moveTo(x, margin.top);
    context.lineTo(x, height - margin.bottom);
    context.stroke();
    context.strokeStyle = "#aeb8c2";
    context.beginPath();
    context.moveTo(x, height - margin.bottom);
    context.lineTo(x, height - margin.bottom + 5);
    context.stroke();
    if (!labelIndexes.has(tickIndex)) return;
    context.fillStyle = "#53606d";
    context.textAlign = tickIndex === 0 ? "left" : tickIndex === xTicks.length - 1 ? "right" : "center";
    context.fillText(tick.label, x, height - margin.bottom + 10);
  });

  const coordinates = points.map((point, index) => ({
    x: xPosition(point.x, index),
    y: yPosition(point.value),
  }));
  state.coordinates = coordinates;
  state.bounds = { left: margin.left, right: width - margin.right, top: margin.top, bottom: height - margin.bottom };

  context.beginPath();
  coordinates.forEach(({ x, y }, index) => {
    if (index === 0) context.moveTo(x, y);
    else context.lineTo(x, y);
  });
  context.lineTo(coordinates.at(-1).x, height - margin.bottom);
  context.lineTo(coordinates[0].x, height - margin.bottom);
  context.closePath();
  context.fillStyle = `${options.color}18`;
  context.fill();

  context.beginPath();
  coordinates.forEach(({ x, y }, index) => {
    if (index === 0) context.moveTo(x, y);
    else context.lineTo(x, y);
  });
  context.strokeStyle = options.color;
  context.lineWidth = 2.5;
  context.lineJoin = "round";
  context.lineCap = "round";
  context.stroke();

  if (coordinates.length <= 100) {
    context.fillStyle = "white";
    context.strokeStyle = options.color;
    context.lineWidth = 1.5;
    coordinates.forEach(({ x, y }) => {
      context.beginPath();
      context.arc(x, y, 2.5, 0, Math.PI * 2);
      context.fill();
      context.stroke();
    });
  }

  if (state.hoverIndex === null || !coordinates[state.hoverIndex]) return;

  const hovered = coordinates[state.hoverIndex];
  const point = points[state.hoverIndex];
  context.save();
  context.setLineDash([4, 4]);
  context.strokeStyle = "#7a8793";
  context.lineWidth = 1;
  context.beginPath();
  context.moveTo(hovered.x, margin.top);
  context.lineTo(hovered.x, height - margin.bottom);
  context.stroke();
  context.restore();

  context.beginPath();
  context.arc(hovered.x, hovered.y, 5, 0, Math.PI * 2);
  context.fillStyle = options.color;
  context.fill();
  context.strokeStyle = "white";
  context.lineWidth = 2;
  context.stroke();

  const valueText = `${point.value.toFixed(options.decimals)} ${options.unit}`;
  context.font = "12px system-ui";
  const tooltipWidth = Math.max(context.measureText(point.label).width, context.measureText(valueText).width) + 24;
  const tooltipHeight = 52;
  let tooltipX = hovered.x + 12;
  if (tooltipX + tooltipWidth > width - 6) tooltipX = hovered.x - tooltipWidth - 12;
  const tooltipY = Math.max(6, Math.min(hovered.y - tooltipHeight - 10, height - tooltipHeight - 6));

  context.fillStyle = "#17212b";
  context.fillRect(tooltipX, tooltipY, tooltipWidth, tooltipHeight);
  context.fillStyle = "white";
  context.textAlign = "left";
  context.textBaseline = "top";
  context.fillText(point.label, tooltipX + 12, tooltipY + 9);
  context.font = "600 12px system-ui";
  context.fillText(valueText, tooltipX + 12, tooltipY + 29);
}

function drawChart(canvasId, points, options) {
  const canvas = document.getElementById(canvasId);
  let state = chartStates.get(canvasId);

  if (!state) {
    const scrollContainer = canvas.closest(".chart-scroll");
    const toolbar = document.querySelector(`[data-controls-for="${canvasId}"]`);
    state = {
      canvas,
      scrollContainer,
      toolbar,
      points: [],
      options: {},
      coordinates: [],
      bounds: null,
      hoverIndex: null,
      zoom: 1,
    };
    chartStates.set(canvasId, state);

    canvas.addEventListener("pointermove", (event) => {
      if (!state.coordinates.length || !state.bounds) return;
      const rectangle = canvas.getBoundingClientRect();
      const pointerX = event.clientX - rectangle.left;
      const pointerY = event.clientY - rectangle.top;
      const inside = pointerX >= state.bounds.left && pointerX <= state.bounds.right
        && pointerY >= state.bounds.top && pointerY <= state.bounds.bottom;
      const nextIndex = inside
        ? state.coordinates.reduce(
          (nearest, coordinate, index) => Math.abs(coordinate.x - pointerX) < Math.abs(state.coordinates[nearest].x - pointerX) ? index : nearest,
          0,
        )
        : null;
      if (nextIndex !== state.hoverIndex) {
        state.hoverIndex = nextIndex;
        renderChart(state);
      }
    });
    canvas.addEventListener("pointerleave", () => {
      if (state.hoverIndex !== null) {
        state.hoverIndex = null;
        renderChart(state);
      }
    });

    canvas.addEventListener("wheel", (event) => {
      if (!event.ctrlKey && !event.metaKey) return;
      event.preventDefault();
      const scrollRectangle = scrollContainer.getBoundingClientRect();
      const focusX = event.clientX - scrollRectangle.left;
      setChartZoom(state, state.zoom * (event.deltaY < 0 ? 1.25 : 0.8), focusX);
    }, { passive: false });

    toolbar.addEventListener("click", (event) => {
      const action = event.target.closest("[data-zoom]")?.dataset.zoom;
      if (action === "in") setChartZoom(state, state.zoom * 1.5);
      if (action === "out") setChartZoom(state, state.zoom / 1.5);
      if (action === "reset") setChartZoom(state, 1);
    });

    if (typeof ResizeObserver !== "undefined") {
      state.resizeObserver = new ResizeObserver(() => resizeChart(state));
      state.resizeObserver.observe(scrollContainer);
    }
  }

  state.points = points;
  state.options = options;
  state.hoverIndex = null;
  resizeChart(state);
}

document.getElementById("power-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const query = new URLSearchParams({
      date: document.getElementById("power-date").value,
      voltage: document.getElementById("power-voltage").value,
    });
    const data = await (await api(`/api/visualizations/instantaneous-power?${query}`)).json();
    document.getElementById("power-summary").textContent = `${data.total_wh.toFixed(2)} Wh in the selected interval`;
    drawChart(
      "power-chart",
      data.points.map((point) => ({
        x: point.timestamp,
        label: hourLabel(point.timestamp, data.timezone),
        value: point.power_watts,
      })),
      {
        color: "#176b55",
        unit: "W",
        decimals: 2,
        xTickInterval: 60 * 60,
        xTickFormatter: (timestamp) => hourLabel(timestamp, data.timezone),
      },
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
    });
    const data = await (await api(`/api/visualizations/daily-consumption?${query}`)).json();
    document.getElementById("consumption-summary").textContent = `${data.total_kwh.toFixed(3)} kWh total`;
    drawChart(
      "consumption-chart",
      data.points.map((point) => ({
        x: point.day,
        label: dayLabel(point.day),
        value: point.consumption_kwh,
      })),
      { color: "#315b9d", unit: "kWh", decimals: 3 },
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
