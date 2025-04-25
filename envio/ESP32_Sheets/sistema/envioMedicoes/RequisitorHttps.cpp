#include "RequisitorHttps.h"

// Construtor
RequisitorHttps::RequisitorHttps(Logador& log, String url, String token): endereco(url), logador(log), token(token) {}

String RequisitorHttps::realizarPOST(const String& dados, int &httpCode) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();  // Ignorar verificação SSL
    http.begin(client, endereco);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    httpCode = http.POST(dados);
    String resposta = http.getString();
    resposta.trim();
    http.end();

    // Verifica se houve redirecionamento para Googleusercontent
    if (resposta.indexOf("Moved Temporarily") != -1) {
        int posInicio = resposta.indexOf("https://script.googleusercontent.com");
        int posFim = resposta.indexOf("\"", posInicio);

        if (posInicio > 0 && posFim > posInicio) {
            String novaURL = resposta.substring(posInicio, posFim);
            novaURL.replace("&amp;", "&");  // Corrige codificação HTML

            // Refaz a requisição GET para a nova URL
            client.setInsecure();
            http.begin(client, novaURL);
            httpCode = http.GET();
            resposta = http.getString();
            resposta.trim();
            http.end();
        }
    }

    return resposta;
}


// Método para verificar conexão corretamente usando POST
bool RequisitorHttps::verificarPOST() {
    int httpCode;
    String resposta = realizarPOST(endereco + "modo=v" + "&token=" + token, httpCode);

    if (resposta == "Dado salvo!") {
        return true;
    } else {
        return false;
    }
}

// Método para adicionar linhas
bool RequisitorHttps::enviarMedicoes(const String& dados) {
    if (dados.isEmpty()) {
        logador.logMensagem("RequisitorHttps: ERRO - Medição vazia.");
        return false;
    }

    String corpo = "modo=al&dados=" + dados + "&token=" + token;
    logador.logMensagem("RequisitorHttps: Enviando via POST: " + corpo);

    int httpCode;
    String resposta = realizarPOST(corpo, httpCode);

    resposta.toLowerCase();
    if (httpCode == 200 && resposta.indexOf("dado salvo") != -1) {
        return true;
    } else {
        logador.logMensagem("RequisitorHttps: ERRO ao adicionar linha. HTTP Code: " +
                              String(httpCode) + ". Resposta: " + resposta);
        return false;
    }
}