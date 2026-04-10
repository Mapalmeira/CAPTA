#include "Medidor.h"

Medidor::Medidor() {}

void Medidor::setResistencia(double r) { resistencia = r; }
void Medidor::setVoltasTransformador(double f) { voltas_transformador = f; }
void Medidor::setOffsetTensao(double offset) { tensao_offset = offset; }
void Medidor::setCoeficientesCorrecao(double angular, double linear) {
    coeficiente_angular = angular;
    coeficiente_linear = linear;
}

Medicao Medidor::medir() {
    int timestamp = getEpoch();
    float corrente = static_cast<float>(medirCorrenteCorrigida());
    return {timestamp, corrente};
}

int Medidor::getEpoch() {
    if (!ntpInicializado) {
        iniciarNTP();
        ntpInicializado = true;
    }

    const int millisAtual = millis();
    struct tm timeinfo;

    if (getLocalTime(&timeinfo, 1000)) {
        epochReferencia = mktime(&timeinfo);
        millisReferencia = millisAtual;
        possuiReferenciaTempo = true;
        return epochReferencia;
    }

    if (possuiReferenciaTempo) {
        const int deltaSegundos = (millisAtual - millisReferencia) / 1000;
        return epochReferencia + deltaSegundos;
    }

    return -1;
}

void Medidor::iniciarNTP() {
    configTime(0, 0, "pool.ntp.org");
}

double Medidor::calcularMedia(int *dados, int tamanho) {
    if (tamanho <= 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += dados[i];
    }
    return soma / tamanho;
}

double Medidor::calcularDesvioPadrao(int *dados, int tamanho, double media) {
    if (tamanho <= 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += pow(dados[i] - media, 2);
    }
    return sqrt(soma / tamanho);
}

int Medidor::filtrarDados(int *dados, int tamanho, double offset, double *dados_filtrados) {
    double media = calcularMedia(dados, tamanho);
    double desvio = calcularDesvioPadrao(dados, tamanho, media);

    int validas = 0;
    for (int i = 0; i < tamanho; i++) {
        if (abs(dados[i] - media) < 2 * desvio) {
            dados_filtrados[validas] = (dados[i] / 4095.0) * 3.3 - offset;
            validas++;
        }
    }
    return validas;
}

double Medidor::calcularRMS(double *dados, int tamanho) {
    if (tamanho == 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += dados[i] * dados[i];
    }
    return sqrt(soma / tamanho);
}

double Medidor::medirCorrenteBruta() {
    int num_leituras = (TEMPO_COLETA * AMOSTRAS_POR_SEGUNDO) / 1000;
    int *leituras = (int *)malloc(num_leituras * sizeof(int));
    double *dados_filtrados = (double *)malloc(num_leituras * sizeof(double));

    if (!leituras || !dados_filtrados) {
        Serial.println("Medidor: erro de alocação de memória.");
        if (leituras) free(leituras);
        if (dados_filtrados) free(dados_filtrados);
        return 0;
    }

    int periodo_us = 1000000 / AMOSTRAS_POR_SEGUNDO;
    int proximo_tempo = micros()
    for (int i = 0; i < num_leituras; i++) {
        proximo_tempo += periodo_us;
        leituras[i] = analogRead(PINO_ANALOGICO);
        while (micros() - proximo_tempo) < 0) {}
    }

    int tamanho_filtrado = filtrarDados(leituras, num_leituras, tensao_offset, dados_filtrados);
    double tensao_RMS = calcularRMS(dados_filtrados, tamanho_filtrado);

    free(leituras);
    free(dados_filtrados);

    return (tensao_RMS / resistencia) * voltas_transformador;
}

double Medidor::medirCorrenteCorrigida() {
    double crms = medirCorrenteBruta();
    return (crms - coeficiente_linear) * coeficiente_angular;
}