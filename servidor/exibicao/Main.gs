function aoEditar(e) {
  const aba = new AbaPrincipal();
  const celulaEditada = e.range.getA1Notation();

  console.log("[Main] onEdit: Célula editada:", celulaEditada);

  const acoes = {
    "D7": PotenciaInstantanea,
    "D8": ConsumoDiario
  };

  if (acoes[celulaEditada]) {
    console.log("[Main] onEdit: Acionando função para", celulaEditada);
    aba.obterRange(celulaEditada).setValue("AQUI");
    acoes[celulaEditada]();
  }
}

function PotenciaInstantanea() {
  const abaPrincipal = new AbaPrincipal();
  const dia = abaPrincipal.obterDia();
  const horaInicial = abaPrincipal.obterHoraInicial();
  const horaFinal = abaPrincipal.obterHoraFinal();
  const voltagem = abaPrincipal.obterVoltagem();

  if (horaFinal > 24 || horaInicial < 0) {
    throw new Error("O intervalo de horas deve estar entre 0 e 24.");
  }

  console.log("[Main] PotenciaInstantanea: Obtendo dados...");
  let dados = Importador.importarPorData(dia);
  if (!dados) {
    throw new Error("Não há dados para o dia selecionado.");
  }

  ProcessadorDeDados.filtrarEDeduplicarDados(dados);
  ProcessadorDeDados.selecionarIntervaloDados(dados, dia, horaInicial, horaFinal);

  console.log("[Main] PotenciaInstantanea: Criando nova aba...");
  const abaPotencia = new AbaPlanilha("Potência instântanea");
  abaPotencia.aba.getRange("A:A").setNumberFormat("hh:mm");
  abaPotencia.ajustarLarguraColunas(3, 300);
  abaPotencia.limpar();

  ProcessadorDeDados.converterPotencia(dados, voltagem);
  const consumoTotal = ProcessadorDeDados.calcularWh(dados);
  abaPotencia.adicionarDadoNaCelula("E3", `${consumoTotal.toFixed(2)} Wh`);
  abaPotencia.adicionarDadosNoRange(`A1:B${dados.length}`, dados);

  GraficoHandler.criarGraficoPotencia(abaPotencia);
  console.log("[Main] PotenciaInstantanea: Gráfico criado com sucesso!");
}

function ConsumoDiario() {
  const abaPrincipal = new AbaPrincipal();
  const dia = abaPrincipal.obterDia();
  const voltagem = abaPrincipal.obterVoltagem();

  console.log("[Main] ConsumoDiario: Obtendo dados...");
  const dados = Importador.importarUltimos30Dias(dia);

  let consumoTotalkWh = 0;
  const dadosProcessados = dados.map(([dia, dadosDia]) => {
    ProcessadorDeDados.filtrarEDeduplicarDados(dadosDia);
    ProcessadorDeDados.selecionarIntervaloDados(dadosDia, dia, 0, 24);
    ProcessadorDeDados.converterPotencia(dadosDia, voltagem);
    const consumoWh = ProcessadorDeDados.calcularWh(dadosDia);
    const consumoKWh = consumoWh / 1000;
    consumoTotalkWh += consumoKWh;
    return [dia, consumoKWh];
  });

  console.log("[Main] ConsumoDiario: Criando nova aba...");
  const abaConsumo = new AbaPlanilha("Consumo diário");
  abaConsumo.ajustarLarguraColunas(3, 300);
  abaConsumo.limpar();

  abaConsumo.adicionarDadoNaCelula("E3", `${consumoTotalkWh.toFixed(3)} kWh`);
  abaConsumo.adicionarDadosNoRange(`A1:B${dadosProcessados.length}`, dadosProcessados);
  abaConsumo.obterRange(`B1:B${dadosProcessados.length}`).setNumberFormat("0.00");

  GraficoHandler.criarGraficoConsumoDiario(abaConsumo);
}