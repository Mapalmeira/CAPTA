#ifndef SISTEMA_H
#define SISTEMA_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "conexao/Conexao.h"
#include "envioMedicoes/EnvioMedicoes.h"
#include "logador/Logador.h"
#include "interfaceFisica/InterfaceFisica.h"

class Sistema {
private:
    // conexão
    String SSID;
    String senha;
    String enderecoAppscript;

    // modo de operação
    String modo;

    // customizações
    bool delayProgressivo = false; // no modo de desconexão, o tempo entre cada medição fica exponencialmente maior até atingir 10 min por medição.
    bool guardarLogs = false;
    bool reiniciarPeriodicamente = false;
    int enviosPorVez = 1;
    int delayAdicional;
    int tempoInicio;

    // objetos
    Conexao conexao;
    Logador logador;
    EnvioMedicoes envioMedicoes;
    InterfaceFisica interfaceFisica;

    // contagem de erros
    int falhasConexao = 0;  // conta as falhas contínuas de conexão.
    int falhasMedicao = 0; // conta as falhas acumuladas de medição.

    // envio
    int ENVIOS_MAXIMOS_POR_VEZ = 50;

public:
    Sistema(String ssid, String senha, String endereco, String token,
        std::function<Medicao(void)> funcaoMedicao,
        std::vector<int> casasDecimais);

    void iniciar();
    void loop();

    // Método de conexão
    bool conectarWIFI();

    // Medição
    bool realizarMedicao();

    // Envio
    String enviaLinhasPlanilha(int linhas);

    // Customizações
    void definirDelayAdicional(int delay);
    void definirDelayProgressivo(bool delay);
    void definirLogs(bool logs);
    void definirReinicializacaoPeriodica(bool reiniciar);
    void definirEnvioPorVez(int envios);

    // Modos
    void definirModo(String modo);
    void modoEnvio();
    void modoPendencia();
    void modoDesconexao();
    void modoReconexao();
    void modoDesligamento();
    void modoReiniciar();
    String getModo();

    // Interação usuario
    void comando(String comando);
};

#endif
