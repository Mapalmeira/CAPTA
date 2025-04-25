function realizarVerificacao(planilha) {
  try {
    const aba = planilha.getSheets()[0];
    aba.getRange("Z1").setValue("teste");
    aba.getRange("Z1").clear();
    return true;
  } catch (e) {
    Logger.log("Erro na verificação: " + e);
    return false;
  }
}

function parseDadosLinha(dadosStr) {
  if (typeof dadosStr !== "string" || dadosStr.trim() === "") {
    throw new Error("A entrada deve ser uma string não vazia.");
  }

  const linhasBrutas = dadosStr.split("_");
  const resultado = [];

  for (let i = 0; i < linhasBrutas.length; i++) {
    const linha = linhasBrutas[i].trim();
    const partes = linha.split("|");

    if (partes.length < 2) {
      throw new Error(`Linha ${i + 1} inválida: deve conter pelo menos um timestamp e um valor.`);
    }

    const timestamp = parseInt(partes[0]);
    if (isNaN(timestamp)) {
      throw new Error(`Linha ${i + 1} inválida: timestamp não é um número válido.`);
    }

    const valores = partes.slice(1).map((v, j) => {
      const num = Number(v);
      if (isNaN(num)) {
        throw new Error(`Linha ${i + 1}, valor ${j + 1}: '${v}' não é um número válido.`);
      }
      return num;
    });

    resultado.push([timestamp, ...valores]);
  }

  return resultado;
}

function obterOuCriarPlanilha(dataStr) {
  const nomeArquivo = dataStr;
  const pasta = obterOuCriarPasta("dadosPart");

  const arquivos = pasta.getFilesByName(nomeArquivo);
  if (arquivos.hasNext()) {
    return SpreadsheetApp.open(arquivos.next());
  }

  const novaPlanilha = SpreadsheetApp.create(nomeArquivo);
  const id = novaPlanilha.getId();
  const arquivo = DriveApp.getFileById(id);
  arquivo.moveTo(pasta);
  return novaPlanilha;
}


function obterOuCriarPasta(nome) {
  const pastas = DriveApp.getFoldersByName(nome);
  if (pastas.hasNext()) {
    return pastas.next();
  }
  return DriveApp.createFolder(nome);
}

function adicionarLinhas(planilha, linhas) {
  const aba = planilha.getSheets()[0];
  aba.getRange(aba.getLastRow() + 1, 1, linhas.length, linhas[0].length).setValues(linhas);
}

function sanitizarDadosStr(str) {
  return str.replace(/[^a-zA-Z0-9._|,-]/g, '');
}