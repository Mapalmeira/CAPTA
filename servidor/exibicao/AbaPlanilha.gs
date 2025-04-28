class AbaPlanilha {
  constructor(nomeAba) {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    this.aba = ss.getSheetByName(nomeAba);

    if (!this.aba) {
      this.aba = ss.insertSheet(nomeAba);
    }

    this.ultimaLinha = this.aba.getLastRow();
  }

  limpar() {
    this.aba.getCharts().forEach(chart => this.aba.removeChart(chart));
    this.aba.clear();
    this.ultimaLinha = 0;
  }

  ajustarLarguraColunas(inicio, largura) {
    const maxCol = this.aba.getMaxColumns();
    for (let i = inicio; i <= maxCol; i++) {
      this.aba.setColumnWidth(i, largura);
    }
  }

  obterValor(celula) {
    return this.aba.getRange(celula).getValue();
  }

  obterRange(celulaOuIntervalo) {
    return this.aba.getRange(celulaOuIntervalo);
  }

  adicionarDadoNaCelula(celula, dado) {
    const range = this.obterRange(celula);
    range.setValue(dado);
  }

  adicionarDadosNoRange(intervalo, dados) {
    const range = this.obterRange(intervalo);
    range.setValues(dados);
  }
}
