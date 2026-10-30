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
  const apiToken = token().trim();
  if (!apiToken) throw new Error("Enter the API token before loading data.");

  let response;
  try {
    response = await fetch(path, {
      headers: { Authorization: `Bearer ${apiToken}` },
    });
  } catch {
    throw new Error("Could not connect to the backend. Check the server and network connection.");
  }

  if (!response.ok) {
    const body = await response.json().catch(() => ({}));
    if (response.status === 401) throw new Error("The API token is invalid or missing.");
    if (response.status === 403) throw new Error("This token cannot access the requested data.");
    if (response.status === 404) throw new Error(body.detail || "No data was found for the selected period.");
    if (response.status === 422 && Array.isArray(body.detail)) {
      const details = body.detail.map((issue) => {
        const field = issue.loc?.at(-1);
        return field ? `${field}: ${issue.msg}` : issue.msg;
      }).join("; ");
      throw new Error(`Invalid request: ${details}`);
    }
    if (response.status >= 500) {
      throw new Error("The backend could not complete the request. Try again shortly.");
    }
    throw new Error(body.detail || `Request failed (${response.status} ${response.statusText})`);
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

function scheduleChartRender(state) {
  if (state.renderFrame !== null) return;

  state.renderFrame = window.requestAnimationFrame(() => {
    state.renderFrame = null;
    renderChart(state);
  });
}

function resizeChart(state) {
  scheduleChartRender(state);
}

function setChartPan(state, nextViewStart) {
  const maximumViewStart = 1 - 1 / state.zoom;
  const viewStart = Math.max(0, Math.min(maximumViewStart, nextViewStart));
  if (viewStart === state.viewStart) return;

  state.viewStart = viewStart;
  scheduleChartRender(state);
}

function setChartZoom(state, nextZoom, focusX = state.canvas.clientWidth / 2) {
  if (!Number.isFinite(nextZoom)) return;

  const zoom = Math.max(1, nextZoom);
  if (zoom === state.zoom) return;

  const viewportWidth = Math.max(state.canvas.clientWidth, 1);
  const focusRatio = Math.max(0, Math.min(1, focusX / viewportWidth));
  const focusedPosition = state.viewStart + focusRatio / state.zoom;
  state.zoom = zoom;
  state.viewStart = Math.max(
    0,
    Math.min(1 - 1 / zoom, focusedPosition - focusRatio / zoom),
  );
  scheduleChartRender(state);
}

function resetChartView(state) {
  state.zoom = 1;
  state.viewStart = 0;
  state.hoverIndex = null;
  updateChartReadout(state);
  scheduleChartRender(state);
}

function touchDistance(touches) {
  return Math.hypot(
    touches[0].clientX - touches[1].clientX,
    touches[0].clientY - touches[1].clientY,
  );
}

function touchFocusX(state, touches) {
  const rectangle = state.canvas.getBoundingClientRect();
  return (touches[0].clientX + touches[1].clientX) / 2 - rectangle.left;
}

function startPinch(state, touches) {
  state.pinch = {
    distance: Math.max(touchDistance(touches), 1),
    zoom: state.zoom,
  };
}

function renderChart(state) {
  const { canvas, points, options } = state;
  const context = canvas.getContext("2d");
  const ratio = window.devicePixelRatio || 1;
  const width = Math.max(canvas.clientWidth, 1);
  const height = 340;
  const compactLayout = width <= 480;
  const margin = {
    top: 28,
    right: compactLayout ? 10 : 22,
    bottom: 48,
    left: compactLayout ? 44 : 66,
  };
  const chartWidth = width - margin.left - margin.right;
  const chartHeight = height - margin.top - margin.bottom;

  canvas.width = Math.round(width * ratio);
  canvas.height = Math.round(height * ratio);
  context.setTransform(ratio, 0, 0, ratio, 0, 0);
  context.clearRect(0, 0, width, height);
  if (!points.length) return;

  const scale = niceScale(Math.max(...points.map((point) => point.value)));
  const regularXInterval = options.xTickInterval;
  const xValues = points.map((point, index) => Number.isFinite(point.x) ? point.x : index);
  const rawMinimumX = Math.min(...xValues);
  const rawMaximumX = Math.max(...xValues);
  let minimumX = regularXInterval
    ? Math.floor(rawMinimumX / regularXInterval) * regularXInterval
    : options.type === "bar" ? rawMinimumX - 0.5 : rawMinimumX;
  let maximumX = regularXInterval
    ? Math.ceil(rawMaximumX / regularXInterval) * regularXInterval
    : options.type === "bar" ? rawMaximumX + 0.5 : rawMaximumX;
  if (minimumX === maximumX) {
    const padding = regularXInterval || 1;
    minimumX -= padding / 2;
    maximumX += padding / 2;
  }
  const fullXSpan = maximumX - minimumX;
  const visibleMinimumX = minimumX + fullXSpan * state.viewStart;
  const visibleMaximumX = visibleMinimumX + fullXSpan / state.zoom;
  const xPosition = (value, index) => {
    const xValue = Number.isFinite(value) ? value : index;
    return margin.left + chartWidth * ((xValue - visibleMinimumX) / (visibleMaximumX - visibleMinimumX));
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
    const firstTick = Math.ceil(visibleMinimumX / regularXInterval) * regularXInterval;
    for (let value = firstTick; value <= visibleMaximumX; value += regularXInterval) {
      xTicks.push({ value, label: options.xTickFormatter(value) });
    }
  } else {
    points.forEach((point, index) => {
      if (index >= visibleMinimumX && index <= visibleMaximumX) {
        xTicks.push({ value: index, label: point.label, pointIndex: index });
      }
    });
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

  const allCoordinates = points.map((point, index) => ({
    x: xPosition(point.x, index),
    y: yPosition(point.value),
    pointIndex: index,
  }));
  const coordinates = allCoordinates.filter(({ x }) => x >= margin.left && x <= width - margin.right);
  state.coordinates = coordinates;
  state.bounds = { left: margin.left, right: width - margin.right, top: margin.top, bottom: height - margin.bottom };

  context.save();
  context.beginPath();
  context.rect(margin.left, margin.top, chartWidth, chartHeight);
  context.clip();

  if (options.type === "bar") {
    const spacing = chartWidth / (visibleMaximumX - visibleMinimumX);
    const barWidth = Math.max(3, Math.min(48, spacing * 0.7));
    context.save();
    context.fillStyle = options.color;
    context.globalAlpha = 0.82;
    coordinates.forEach(({ x, y }) => {
      context.fillRect(x - barWidth / 2, y, barWidth, height - margin.bottom - y);
    });
    context.restore();
  } else {
    context.beginPath();
    allCoordinates.forEach(({ x, y }, index) => {
      if (index === 0) context.moveTo(x, y);
      else context.lineTo(x, y);
    });
    context.lineTo(allCoordinates.at(-1).x, height - margin.bottom);
    context.lineTo(allCoordinates[0].x, height - margin.bottom);
    context.closePath();
    context.fillStyle = `${options.color}18`;
    context.fill();

    context.beginPath();
    allCoordinates.forEach(({ x, y }, index) => {
      if (index === 0) context.moveTo(x, y);
      else context.lineTo(x, y);
    });
    context.strokeStyle = options.color;
    context.lineWidth = 2.5;
    context.lineJoin = "round";
    context.lineCap = "round";
    context.stroke();

    if (allCoordinates.length <= 100) {
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
  }

  context.restore();

  const hovered = coordinates.find(({ pointIndex }) => pointIndex === state.hoverIndex);
  if (hovered) {
    context.save();
    context.beginPath();
    context.rect(margin.left, margin.top, chartWidth, chartHeight);
    context.clip();

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
    context.restore();
  }

  context.strokeStyle = "#8e9aa6";
  context.lineWidth = 1;
  context.beginPath();
  context.moveTo(margin.left, margin.top);
  context.lineTo(margin.left, height - margin.bottom);
  context.lineTo(width - margin.right, height - margin.bottom);
  context.stroke();

  if (!hovered) return;

  const point = points[state.hoverIndex];
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

function updateChartReadout(state) {
  if (!state.readout) return;

  const point = state.points[state.hoverIndex];
  state.readout.value = point
    ? `${point.label}: ${point.value.toFixed(state.options.decimals)} ${state.options.unit}`
    : "";
}

function updateHoveredPoint(state, event) {
  if (!state.coordinates.length || !state.bounds) return;

  const rectangle = state.canvas.getBoundingClientRect();
  const pointerX = event.clientX - rectangle.left;
  const pointerY = event.clientY - rectangle.top;
  const inside = pointerX >= state.bounds.left && pointerX <= state.bounds.right
    && pointerY >= state.bounds.top && pointerY <= state.bounds.bottom;
  const nextIndex = inside
    ? state.coordinates.reduce((nearest, coordinate) => (
      Math.abs(coordinate.x - pointerX) < Math.abs(nearest.x - pointerX)
        ? coordinate
        : nearest
    )).pointIndex
    : null;
  if (nextIndex !== state.hoverIndex) {
    state.hoverIndex = nextIndex;
    updateChartReadout(state);
    scheduleChartRender(state);
  }
}

function drawChart(canvasId, points, options) {
  const canvas = document.getElementById(canvasId);
  let state = chartStates.get(canvasId);

  if (!state) {
    const scrollContainer = canvas.closest(".chart-scroll");
    const readout = document.querySelector(`[data-value-for="${canvasId}"]`);
    state = {
      canvas,
      scrollContainer,
      readout,
      points: [],
      options: {},
      coordinates: [],
      bounds: null,
      hoverIndex: null,
      zoom: 1,
      viewStart: 0,
      pinch: null,
      touchStart: null,
      mouseDrag: null,
      renderFrame: null,
    };
    chartStates.set(canvasId, state);

    canvas.addEventListener("pointermove", (event) => {
      if (event.pointerType === "touch") return;
      if (state.mouseDrag?.pointerId === event.pointerId) {
        const deltaX = event.clientX - state.mouseDrag.x;
        if (Math.abs(deltaX) > 3) state.mouseDrag.moved = true;
        if (state.mouseDrag.moved) {
          setChartPan(
            state,
            state.mouseDrag.viewStart - deltaX / Math.max(canvas.clientWidth * state.zoom, 1),
          );
        }
        return;
      }
      updateHoveredPoint(state, event);
    });
    canvas.addEventListener("pointerleave", (event) => {
      if (event.pointerType === "touch") return;
      if (state.mouseDrag) return;
      if (state.hoverIndex !== null) {
        state.hoverIndex = null;
        updateChartReadout(state);
        scheduleChartRender(state);
      }
    });
    canvas.addEventListener("pointerdown", (event) => {
      if (event.pointerType !== "mouse" || event.button !== 0 || state.zoom === 1) return;
      state.mouseDrag = {
        pointerId: event.pointerId,
        x: event.clientX,
        viewStart: state.viewStart,
        moved: false,
      };
      canvas.setPointerCapture(event.pointerId);
    });
    canvas.addEventListener("pointerup", (event) => {
      if (state.mouseDrag?.pointerId !== event.pointerId) return;
      const moved = state.mouseDrag.moved;
      state.mouseDrag = null;
      canvas.releasePointerCapture(event.pointerId);
      if (!moved) updateHoveredPoint(state, event);
    });
    canvas.addEventListener("pointercancel", () => {
      state.mouseDrag = null;
    });
    canvas.addEventListener("wheel", (event) => {
      if (!event.ctrlKey) return;
      event.preventDefault();
      const rectangle = canvas.getBoundingClientRect();
      const factor = Math.exp(-event.deltaY * 0.002);
      setChartZoom(state, state.zoom * factor, event.clientX - rectangle.left);
    }, { passive: false });
    canvas.addEventListener("dblclick", () => resetChartView(state));
    scrollContainer.addEventListener("keydown", (event) => {
      if (event.key === "+" || event.key === "=") {
        event.preventDefault();
        setChartZoom(state, state.zoom * 1.25);
      } else if (event.key === "-" || event.key === "_") {
        event.preventDefault();
        setChartZoom(state, state.zoom / 1.25);
      } else if (event.key === "0") {
        event.preventDefault();
        resetChartView(state);
      } else if (event.key === "ArrowLeft" || event.key === "ArrowRight") {
        event.preventDefault();
        const direction = event.key === "ArrowLeft" ? -1 : 1;
        setChartPan(state, state.viewStart + direction * 0.1 / state.zoom);
      }
    });

    canvas.addEventListener("touchstart", (event) => {
      if (event.touches.length === 1) {
        state.touchStart = {
          x: event.touches[0].clientX,
          y: event.touches[0].clientY,
          viewStart: state.viewStart,
          panning: false,
          select: true,
        };
        return;
      }
      if (event.touches.length !== 2) return;
      state.touchStart = null;
      event.preventDefault();
      startPinch(state, event.touches);
    }, { passive: false });

    canvas.addEventListener("touchmove", (event) => {
      if (event.touches.length === 1) {
        if (state.touchStart) {
          const deltaX = event.touches[0].clientX - state.touchStart.x;
          const deltaY = event.touches[0].clientY - state.touchStart.y;
          if (Math.hypot(deltaX, deltaY) > 14) state.touchStart.select = false;
          if (Math.abs(deltaX) > 14 && Math.abs(deltaX) > Math.abs(deltaY)) {
            state.touchStart.panning = true;
          }
          if (state.touchStart.panning && state.zoom > 1) {
            event.preventDefault();
            setChartPan(
              state,
              state.touchStart.viewStart - deltaX / Math.max(canvas.clientWidth * state.zoom, 1),
            );
          }
        }
        return;
      }
      if (event.touches.length !== 2) return;
      state.touchStart = null;
      event.preventDefault();
      if (!state.pinch) startPinch(state, event.touches);
      const distanceRatio = touchDistance(event.touches) / state.pinch.distance;
      const nextZoom = state.pinch.zoom * distanceRatio ** 2;
      setChartZoom(state, nextZoom, touchFocusX(state, event.touches));
    }, { passive: false });

    canvas.addEventListener("touchend", (event) => {
      if (event.touches.length < 2) state.pinch = null;
      if (state.touchStart?.select && event.changedTouches.length === 1) {
        updateHoveredPoint(state, event.changedTouches[0]);
      }
      state.touchStart = null;
    });
    canvas.addEventListener("touchcancel", () => {
      state.pinch = null;
      state.touchStart = null;
    });

    if (typeof ResizeObserver !== "undefined") {
      state.resizeObserver = new ResizeObserver(() => resizeChart(state));
      state.resizeObserver.observe(scrollContainer);
    }
  }

  state.points = points;
  state.options = options;
  state.zoom = 1;
  state.viewStart = 0;
  state.hoverIndex = null;
  updateChartReadout(state);
  resizeChart(state);
}

function clearChart(canvasId) {
  const state = chartStates.get(canvasId);
  if (!state) return;

  state.points = [];
  state.coordinates = [];
  state.bounds = null;
  state.hoverIndex = null;
  updateChartReadout(state);
  scheduleChartRender(state);
}

document.getElementById("power-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  const errorOutput = document.getElementById("power-error");
  errorOutput.textContent = "";
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
    document.getElementById("power-summary").textContent = "";
    clearChart("power-chart");
    errorOutput.textContent = error.message;
  }
});

document.getElementById("consumption-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  const errorOutput = document.getElementById("consumption-error");
  errorOutput.textContent = "";
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
      { color: "#176b55", unit: "kWh", decimals: 3, type: "bar" },
    );
  } catch (error) {
    document.getElementById("consumption-summary").textContent = "";
    clearChart("consumption-chart");
    errorOutput.textContent = error.message;
  }
});

document.getElementById("export-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  const errorOutput = document.getElementById("export-error");
  errorOutput.textContent = "";
  try {
    const day = document.getElementById("export-date").value;
    const blob = await (await api(`/api/measurements/${day}/export`)).blob();
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = `${day}.csv`;
    link.click();
    URL.revokeObjectURL(link.href);
  } catch (error) {
    errorOutput.textContent = error.message;
  }
});
