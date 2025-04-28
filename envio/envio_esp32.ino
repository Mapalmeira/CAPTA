// Includes de headers
#include "sistema/Sistema.h"
#include "medidor/Medidor.h"
#include "sistema/InterfaceFisica/InterfaceFisica.h"
#include "sistema/envioMedicoes/EnvioMedicoes.h"
#include "sistema/envioMedicoes/RequisitorHttps.h"
#include "sistema/logador/Logador.h"
#include "sistema/conexao/Conexao.h"

// Includes de implementação
#include "sistema/Sistema.cpp"
#include "medidor/Medidor.cpp"
#include "sistema/InterfaceFisica/InterfaceFisica.cpp"
#include "sistema/envioMedicoes/EnvioMedicoes.cpp"
#include "sistema/envioMedicoes/RequisitorHttps.cpp"
#include "sistema/logador/Logador.cpp"
#include "sistema/conexao/Conexao.cpp"

// Configurações do sistema
constexpr const char* SSID = "";
constexpr const char* PASSWORD = "";
constexpr const char* ENDERECO_SHEETS = "";
constexpr const char* TOKEN = "";
constexpr bool ATIVAR_LOGS = true;  // ainda não implementado
constexpr bool REINICIALIZACAO_PERIODICA = true;
constexpr int ENVIOS_POR_VEZ = 10;
constexpr int DELAY_ADICIONAL = 7500;
constexpr bool DELAY_PROGRESSIVO = false;
const std::vector<int> CASAS_DECIMAIS = {-1, 2};

// Configurações do medidor
constexpr double RESISTENCIA = 75;
constexpr double VOLTAS_TRANSFORMADOR = 2000.0;
constexpr double TENSAO_OFFSET = 1.65;
constexpr double COEFICIENTE_ANGULAR = 1;
constexpr double COEFICIENTE_LINEAR = -0;

Medidor medidor;

auto funcMedicao = [&medidor]() {
    return medidor.medir();
};

Sistema sistema(
    SSID,
    PASSWORD,
    ENDERECO_SHEETS,
    TOKEN,
    funcMedicao,
    CASAS_DECIMAIS // casas decimais para timestamp e amperagem. -1 significa inteiro.
);

void setup() {
    Serial.begin(115200);
    delay(4000);

    // configurações do medidor
    medidor.setResistencia(RESISTENCIA);
    medidor.setVoltasTransformador(VOLTAS_TRANSFORMADOR);
    medidor.setOffsetTensao(TENSAO_OFFSET);
    medidor.setCoeficientesCorrecao(COEFICIENTE_ANGULAR, COEFICIENTE_LINEAR);

    // configurações opcionais do sistema
    sistema.definirLogs(ATIVAR_LOGS);
    sistema.definirReinicializacaoPeriodica(REINICIALIZACAO_PERIODICA);
    sistema.definirEnvioPorVez(ENVIOS_POR_VEZ);
    sistema.definirDelayAdicional(DELAY_ADICIONAL);
    sistema.definirDelayProgressivo(DELAY_PROGRESSIVO);

    // inicia o sistema
    sistema.iniciar();
}

void loop() {
    // verifica entrada serial para comandos
    if (Serial.available()) {
        String comando = Serial.readStringUntil('\n');
        sistema.comando(comando);
    }

    // executa o sistema de medidas e envio
    sistema.loop();
    delay(100);
}
