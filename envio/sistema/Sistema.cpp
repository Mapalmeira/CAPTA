#include "Sistema.h"

// Construtor
Sistema::Sistema(String ssid, String senha, String endereco, String token,
                 std::function<Medicao(void)> funcaoMedicao,
                 std::vector<int> casasDecimais)
    : SSID(ssid),
      senha(senha),
      enderecoPlanilha(endereco),
      conexao(ssid, senha),
      envioMedicoes(endereco, logador, token, funcaoMedicao, casasDecimais),
      interfaceFisica(),
      modo("envio")
{}


void Sistema::iniciar() {
    Serial.println("Sistema: Iniciando...");
    
    // iniciar logador
    logador.iniciar(this->guardarLogs);
    
    // Inicia o wi-fi
    conectarWIFI();
    logador.logMensagem(conexao.exibirStatus());

    // Inicia a Interface Fisica
    auto getModoFunc = [this]() { return this->getModo(); };
    auto comandoFunc = [this](String cmd) { this->comando(cmd); };
    interfaceFisica.iniciar(getModoFunc, comandoFunc);
    
    this->tempoInicio = millis();
}


// Conectar ao Wi-Fi
bool Sistema::conectarWIFI() {
    bool conectou = conexao.conectarWIFI();
    if (conectou) {
        falhasConexao = 0;
    } else {
        falhasConexao += 1;
    }
    return conectou;
}

bool Sistema::realizarMedicao() {
    bool mediu = envioMedicoes.realizarMedicao();
    if (!mediu) {
        falhasMedicao += 1;
    }

    delay(delayAdicional);
    return mediu;
}

// Envio
String Sistema::enviaLinhasPlanilha(int linhas) {
    String status = envioMedicoes.enviarLinhasPOST(linhas);
    if (status == "erro medicao") {
        falhasMedicao += 1;
    } else if (status == "erro conexao") {
        falhasConexao += 1;
    } else if (status == "enviado") {
        falhasConexao = 0;
    }

    return status;
}

// Customizações
void Sistema::definirDelayAdicional(int delay) {
    this->delayAdicional = delay;
}

void Sistema::definirDelayProgressivo(bool delay) {
    this->delayProgressivo = delay;
}

void Sistema::definirLogs(bool logs) {
    this->guardarLogs = logs;
}

void Sistema::definirReinicializacaoPeriodica(bool reiniciar) {
    this->reiniciarPeriodicamente = reiniciar;
}

void Sistema::definirEnvioPorVez(int envios) {
    this->enviosPorVez = envios;
}

// Modos
void Sistema::definirModo(String modo) {
    if (modo == "envio" || modo == "pendencia" || modo == "desconexao" || modo == "desligamento" || modo == "reiniciar") {
        this->modo = modo;
        return;
    }
    logador.logMensagem("Sistema: modo inválido!");    
}

void Sistema::loop() {
    // lógica para trocar entre os modos.
    if (falhasConexao >= 3 && !conexao.verificaStatus()) {modo = "desconexao";}
    if (modo == "desconexao" && conexao.verificaStatus()) {modo = "envio";}
    if (modo == "envio" && envioMedicoes.getTamanhoBuffer() >= ENVIOS_MAXIMOS_POR_VEZ) {modo = "pendencia";}
    if (modo == "pendencia" && envioMedicoes.getTamanhoBuffer() == 0) {modo = "envio";}

    // reiniciar
    if (falhasMedicao >= 3) {modo = "reiniciar";} // as medições nunca deveriam falhar.
    if (falhasConexao >= 3 && conexao.verificaStatus() && !envioMedicoes.statusPOST()) {modo = "reiniciar";}  // se a conexão está em pé e mesmo assim há erros, então há algo muito errado com a planilha.
    if (reiniciarPeriodicamente && (millis() - tempoInicio >= 1 * 60 * 60 * 1000)) {modo = "reiniciar";}

    // printando informações úteis
    logador.logMensagem("\nSistema: modo '" + modo + "'");
    Serial.println("Sistema: Pacotes restantes no buffer: " + String(envioMedicoes.getTamanhoBuffer()));

    // executando os modos.
    if (modo == "envio") {modoEnvio();}
    else if (modo == "pendencia") {modoPendencia();}
    else if (modo == "desconexao") {modoDesconexao();}
    else if (modo == "desligamento") {modoDesligamento();}
    else if (modo == "reiniciar") {modoReiniciar();}
    else {
        logador.logMensagem("Sistema: modo de operação inválido. Alterando para modo de envio padrão.");
        modo = "envio";
    }
}

void Sistema::modoEnvio() {
    realizarMedicao();

    if (envioMedicoes.getTamanhoBuffer() >= enviosPorVez) {
        enviaLinhasPlanilha(ENVIOS_MAXIMOS_POR_VEZ);
    }   
}

void Sistema::modoPendencia() {
    enviaLinhasPlanilha(ENVIOS_MAXIMOS_POR_VEZ);
}

void Sistema::modoDesconexao() {
    realizarMedicao();
    if (conectarWIFI()) {
        falhasConexao = 0;
    }

    if (delayProgressivo) {
        if (falhasConexao < 50) {
          delay(1000);
        } else if (falhasConexao < 100) {
            delay(5000);
        } else if (falhasConexao < 150) {
            delay(25000);
        } else if (falhasConexao < 200) {
            delay(120000);
        } else if (falhasConexao < 250) {
            delay(300000);
        } else {
            delay(600000);
        }
    }
}

void Sistema::modoDesligamento() {
    enviaLinhasPlanilha(ENVIOS_MAXIMOS_POR_VEZ);
    logador.logMensagem("Sistema: Desligando ESP32...");
    delay(1000);
    esp_deep_sleep_start();
}

void Sistema::modoReiniciar() {
    enviaLinhasPlanilha(ENVIOS_MAXIMOS_POR_VEZ);
    logador.logMensagem("Sistema: Reiniciando ESP32...");
    delay(1000);
    ESP.restart();
}

String Sistema::getModo() {
    return modo;
}

// Interação com usuário
void Sistema::comando(String comando) {
    comando.trim();

    if (comando.equalsIgnoreCase("stop")) {
        modo = "desligamento";
    } else {
        logador.logMensagem("Sistema: Comando desconhecido.");
    }
}



