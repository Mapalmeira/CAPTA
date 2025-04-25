class Importador {
  static importarPorData(data) {
    const nomeBase = Utilities.formatDate(data, Session.getScriptTimeZone(), "yyyy-MM-dd");

    // Tenta importar de dadosWhole/ primeiro
    const pastaCsv = Importador._obterPastaPorNome("dadosWhole");
    const arquivoCsv = Importador._obterArquivoPorNome(pastaCsv, `${nomeBase}.csv`);

    if (arquivoCsv) {
      console.log(`[Importador] Importando de CSV: ${nomeBase}.csv`);
      return Importador._lerCsvComoArray(arquivoCsv);
    }

    // Se não encontrou o CSV, tenta importar de dadosPart/
    const pastaPlanilhas = Importador._obterPastaPorNome("dadosPart");
    const arquivoPlanilha = Importador._obterArquivoPorNome(pastaPlanilhas, nomeBase);

    if (arquivoPlanilha) {
      console.log(`[Importador] Importando de planilha: ${nomeBase}`);
      return Importador._lerPlanilhaComoArray(arquivoPlanilha);
    }

    // Nenhuma fonte encontrada
    return null;
  }

  static importarUltimos30Dias(dia) {
    const dias = [];
    
    for (let i = 30; i >= 0; i--) {
      const dataAtual = new Date(dia);
      dataAtual.setDate(dia.getDate() - i);
      
      const dados = Importador.importarPorData(dataAtual);
      if (dados !== null) {
        dias.push([dataAtual, dados]);
      }
    }

    return dias;
  }

  // === Métodos auxiliares ===

  static _obterPastaPorNome(nome) {
    const pastas = DriveApp.getFoldersByName(nome);
    if (!pastas.hasNext()) {
      throw new Error(`[Importador] Pasta "${nome}" não encontrada.`);
    }
    return pastas.next();
  }

  static _obterArquivoPorNome(pasta, nomeArquivo) {
    const arquivos = pasta.getFilesByName(nomeArquivo);
    return arquivos.hasNext() ? arquivos.next() : null;
  }

  static _lerCsvComoArray(arquivo) {
    const conteudo = arquivo.getBlob().getDataAsString();

    if (typeof conteudo !== "string" || conteudo.trim() === "") {
      throw new Error("[Importador] O conteúdo do arquivo CSV está vazio ou inválido.");
    }

    const linhas = Utilities.parseCsv(conteudo);

    if (!Array.isArray(linhas)) {
      throw new Error("[Importador] O conteúdo parseado não é uma lista de linhas.");
    }

    const linhasValidas = [];
    for (let i = 0; i < linhas.length; i++) {
      const linha = linhas[i];

      if (!Array.isArray(linha) || linha.length < 2) {
        console.warn(`[Importador] Linha ${i + 1} ignorada: formato inválido.`);
        continue;
      }

      const timestamp = parseInt(linha[0], 10);
      const valor = Number(linha[1]);

      if (isNaN(timestamp) || isNaN(valor)) {
        console.warn(`[Importador] Linha ${i + 1} ignorada: timestamp ou valor não numérico.`);
        continue;
      }

      linhasValidas.push([timestamp, valor]);
    }

    return linhasValidas;
  }

  static _lerPlanilhaComoArray(arquivo) {
    const planilha = SpreadsheetApp.open(arquivo);
    const aba = planilha.getSheets()[0];
    const dados = aba.getDataRange().getValues();

    if (!Array.isArray(dados)) {
      throw new Error("[Importador] A leitura da planilha não retornou uma matriz de dados.");
    }

    const linhasValidas = [];
    for (let i = 0; i < dados.length; i++) {
      const linha = dados[i];

      if (!Array.isArray(linha) || linha.length < 2) {
        console.warn(`[Importador] Linha ${i + 1} ignorada: menos de 2 colunas.`);
        continue;
      }

      const timestamp = parseInt(linha[0], 10);
      const valor = Number(linha[1]);

      if (isNaN(timestamp) || isNaN(valor)) {
        console.warn(`[Importador] Linha ${i + 1} ignorada: timestamp ou valor inválido.`);
        continue;
      }

      linhasValidas.push([timestamp, valor]);
    }

    return linhasValidas;
  }
}
