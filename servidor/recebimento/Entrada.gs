const TOKEN_SECRETO = "";

function doPost(e) {
  const token = e.parameter.token;
  if (!validarToken(token)) {
    return ContentService.createTextOutput("Acesso negado");
  }

  const modo = e.parameter.modo;
  if (modo === "csv") return modoRecebimentoWhole(e);
  if (modo === "al") return modoAdicaoLinhas(e);
  if (modo === "v") return modoVerificacao();

  return ContentService.createTextOutput("Modo inválido.");
}

// Validação de token unificada
function validarToken(token) {
  return token === TOKEN_SECRETO;
}