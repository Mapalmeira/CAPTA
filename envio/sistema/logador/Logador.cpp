#include "Logador.h"

Logador::Logador() {}

// PARA IMPLEMENTAÇÃO FUTURA

void Logador::iniciar(boolean guardarLogs) {
    if (!guardarLogs) { 
        return;
    }
}

void Logador::logMensagem(String mensagem) {
    // Exibe a mensagem no Serial Monitor
    Serial.println(mensagem);
}