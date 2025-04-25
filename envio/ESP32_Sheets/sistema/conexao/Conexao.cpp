#include "Conexao.h"

Conexao::Conexao(String ssid, String senha) 
    : rede_wifi(ssid), senha_wifi(senha) {}

bool Conexao::verificaStatus() {
    return WiFi.status() == WL_CONNECTED;
}

bool Conexao::conectarWIFI() {
    WiFi.disconnect();
    WiFi.begin(rede_wifi.c_str(), senha_wifi.c_str());

    unsigned long tempoInicial = millis();
    const unsigned long timeout = 10000;

    while (!verificaStatus()) {
        delay(500);
        Serial.print(".");
        if (millis() - tempoInicial > timeout) {
            return false;
        }
    }
    return true;
}

String Conexao::exibirStatus() {
  if (verificaStatus()) {
    return "\nConexão: Wi-Fi conectado! Endereço IP: " + String(WiFi.localIP().toString());
  } else {
    return "Conexão: Wi-Fi desconectado";
  }
}