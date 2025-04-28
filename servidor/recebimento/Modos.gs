function modoAdicaoLinhas(e) {
  const dadosStr = sanitizarDadosStr(e.parameter.dados);  
  const dados = parseDadosLinha(dadosStr);
  const timestampInicial = dados[0][0] * 1000;
  const data = new Date(timestampInicial);

  const dataFormatada = data.getFullYear() + '-' +
                        String(data.getMonth() + 1).padStart(2, '0') + '-' +
                        String(data.getDate()).padStart(2, '0');

  const planilha = obterOuCriarPlanilha(dataFormatada);
  adicionarLinhas(planilha, dados);

  return ContentService.createTextOutput("Dado salvo!");
}

function modoVerificacao() {
  const dataAtual = new Date().toISOString().substring(0, 10);
  const planilha = verificarOuCriarPlanilha(dataAtual);

  const sucesso = realizarVerificacao(planilha);
  return ContentService.createTextOutput(sucesso ? "Dado salvo!" : "Erro ao verificar.");
}

function modoRecebimentoWhole(e) {
  if (!e.postData) {
    throw new Error("postData indefinido");
  }

  let nomeArquivo = e.parameter.nome;
  if (!nomeArquivo) {
    throw new Error("Parâmetro 'nome' ausente na URL");
  }
  nomeArquivo = sanitizarDadosStr(nomeArquivo);

  const conteudoCSV = e.postData.contents;
  const pasta = obterOuCriarPasta("dadosWhole");
  const blob = Utilities.newBlob(conteudoCSV, "text/csv", nomeArquivo);
  pasta.createFile(blob);

  return ContentService.createTextOutput("Dado Salvo!");
}