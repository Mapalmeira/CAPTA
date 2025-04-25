class GraficoHandler {
  static criarGraficoPotencia(abaPotencia) {
    console.log("[GraficoHandler] criarGraficoPotencia: Iniciando...");

    const aba = abaPotencia.aba;
    const ultimaLinha = aba.getLastRow();

    if (ultimaLinha < 2) {
      console.warn("[GraficoHandler] criarGraficoPotencia: Poucos dados para gerar gráfico.");
      return;
    }

    const dataInicial = abaPotencia.obterValor("A1");
    const dataFinal = abaPotencia.obterValor(`A${ultimaLinha}`);
    const horasTotais = Math.min(Math.ceil((dataFinal - dataInicial) / 3600000), 24);

    const altura = 600;
    const larguraMinima = 1600;
    const larguraMaxima = 4000;
    const larguraPorHora = 150;
    const largura = Math.min(larguraMaxima, Math.max(larguraMinima, horasTotais * larguraPorHora));

    const chart = aba.newChart()
      .setChartType(Charts.ChartType.LINE)
      .addRange(aba.getRange(`A1:B${ultimaLinha}`))
      .setPosition(5, 3, 10, 10)
      .setOption("title", "Gráfico de Potência (W)")
      .setOption("hAxis", {
        title: "Hora",
        format: "HH:mm",
        slantedText: false,
        gridlines: { count: Math.max(horasTotais, 2) },
        viewWindow: { min: dataInicial, max: dataFinal }
      })
      .setOption("vAxis", {
        title: "Potência (W)",
        viewWindow: { min: 0 },
        gridlines: { count: Math.ceil(8000 / 500) }
      })
      .setOption("width", largura)
      .setOption("height", altura)
      .build();

    aba.insertChart(chart);
    console.log("[GraficoHandler] criarGraficoPotencia: Gráfico inserido com sucesso.");
  }

  static criarGraficoConsumoDiario(abaConsumo) {
    console.log("[GraficoHandler] criarGraficoConsumoDiario: Iniciando...");

    const aba = abaConsumo.aba;
    const ultimaLinha = aba.getLastRow();

    if (ultimaLinha < 2) {
      console.warn("[GraficoHandler] criarGraficoConsumoDiario: Poucos dados para gerar gráfico.");
      return;
    }

    const altura = 600;
    const largura = 1200;

    const valores = aba.getRange(`B2:B${ultimaLinha}`).getValues().flat();
    const maxValor = Math.max(...valores);
    const maxArredondado = Math.ceil(maxValor / 5) * 5;

    const chart = aba.newChart()
      .setChartType(Charts.ChartType.COLUMN)
      .addRange(aba.getRange(`A1:B${ultimaLinha}`))
      .setPosition(5, 3, 10, 10)
      .setOption("title", "Consumo Diário (kWh)")
      .setOption("hAxis", {
        title: "Dia",
        format: "dd/MM",
        slantedText: true
      })
      .setOption("vAxis", {
        title: "Consumo (kWh)",
        viewWindow: { min: 0, max: maxArredondado },
        gridlines: { count: maxArredondado / 5 + 1}
      })
      .setOption("width", largura)
      .setOption("height", altura)
      .build();

    aba.insertChart(chart);
    console.log("[GraficoHandler] criarGraficoConsumoDiario: Gráfico de colunas inserido com sucesso.");
  }
}
