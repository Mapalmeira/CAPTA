class ProcessadorDeDados {
  static filtrarEDeduplicarDados(dados) {
    const mapaDados = new Map();

    for (const linha of dados) {
      const timestamp = linha[0];

      if (timestamp === "" || timestamp === null) {
        console.warn("[ProcessadorDeDados] filtrarEDeduplicarDados: Ignorando linha vazia.");
        continue;
      }
      if (timestamp === -1) {
        console.warn(`[ProcessadorDeDados] filtrarEDeduplicarDados: Removendo timestamp inválido (-1) -> ${linha}`);
        continue;
      }
      if (!mapaDados.has(timestamp)) {
        mapaDados.set(timestamp, linha);
      } else {
        console.warn(`[ProcessadorDeDados] filtrarEDeduplicarDados: Removendo duplicata -> ${linha}`);
      }
    }

    const dadosOrdenados = Array.from(mapaDados.values()).sort((a, b) => a[0] - b[0]);
    dados.length = 0;
    dados.push(...dadosOrdenados);

    console.log(`[ProcessadorDeDados] ${dados.length} linhas válidas após limpeza e ordenação.`);
  }

  static selecionarIntervaloDados(dados, dia, horaInicial, horaFinal) {
    console.log("[ProcessadorDeDados] selecionarIntervaloDados: Iniciando...");

    if (dados.length === 0) {
      console.error("[ProcessadorDeDados] ERRO - Nenhum dado disponível!");
      throw new Error("Nenhum dado disponível para o período selecionado.");
    }

    // Obtendo timestamp do intervalo
    const dataInicio = new Date(dia.getTime());
    dataInicio.setHours(horaInicial, 0, 1, 0);
    const dataFim = new Date(dia.getTime());
    dataFim.setHours(horaFinal, 0, 0, 0);
    dataFim.setTime(dataFim.getTime() - 1);
    const timestampInicio = Math.floor(dataInicio.getTime() / 1000);
    const timestampFim = Math.floor(dataFim.getTime() / 1000);

    // Filtrando para o intervalo
    const filtrados = dados
      .filter(([timestamp]) => timestamp >= timestampInicio && timestamp <= timestampFim);

    if (filtrados.length === 0) {
      throw new Error("Nenhum dado no intervalo de horário definido.");
    }

    // Adicionando pontos limítrofes
    const valorInicial = filtrados[0][1];
    const valorFinal = filtrados[filtrados.length - 1][1];

    filtrados.unshift([timestampInicio, valorInicial]);
    filtrados.push([timestampFim, valorFinal]);

    console.log("[GraficoHandler] Pontos limítrofes adicionados.");

    // Efeito colateral
    dados.length = 0;
    dados.push(...filtrados);
    console.log(`[GraficoHandler] prepararDadosParaGraficoPotencia: ${dados.length} pontos prontos para gráfico.`);
  }

  static converterPotencia(dados, voltagem) {
    const convertidos = dados.map(([timestamp, valor]) => [new Date(timestamp * 1000), valor * voltagem]);
    dados.length = 0;
    dados.push(...convertidos);
  }

  static calcularWh(dados) {
    if (dados.length < 2) {
      throw new Error("[ProcessadorDeDados] calcularWh: Dados insuficientes para calcular consumo.");
    }

    console.log(`[ProcessadorDeDados] calcularWh: Número de medições usadas: ${dados.length}`);

    const tempo = dados.map(item => item[0].getTime() / 1000);
    const potencia = dados.map(item => item[1]);

    console.log(`[ProcessadorDeDados] calcularWh: Intervalo de tempo detectado - Primeiro: ${tempo[0]}, Último: ${tempo[tempo.length - 1]}`);

    const consumoWs = Calculadora.integral(tempo, potencia);
    console.log(`[ProcessadorDeDados] calcularWh: Consumo calculado em Ws: ${consumoWs.toFixed(2)} Ws`);

    const consumoWh = consumoWs / 3600;
    console.log(`[ProcessadorDeDados] calcularWh: Consumo final: ${consumoWh.toFixed(2)} Wh`);
    return consumoWh;
  }
}