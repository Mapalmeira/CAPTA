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

void Medidor::iniciarI2C() {
    Wire.begin();
    Wire.setClock(400000);
  
    if (!ads.begin()) {
      Serial.println("Medidor: ao iniciar ADS1115!");
      return;
    }

    ads.setGain(GAIN_ONE);
    ads.setDataRate(RATE_ADS1115_860SPS);
}

double Medidor::calcularMedia(double *dados, int tamanho) {
    if (tamanho <= 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += dados[i];
    }
    return soma / tamanho;
}

double Medidor::calcularDesvioPadrao(double *dados, int tamanho, double media) {
    if (tamanho <= 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
      soma += pow(dados[i] - media, 2);
    }
    return sqrt(soma / tamanho);
}

int Medidor::filtrarDados(double *dados, int tamanho, double offset, double *filtrados) {
    double media = calcularMedia(dados, tamanho);
    double desvio = calcularDesvioPadrao(dados, tamanho, media);

    int validos = 0;
    for (int i = 0; i < tamanho; i++) {
      if (abs(dados[i] - media) < 2 * desvio) {
        double tensao = dados[i] - offset;
        filtrados[validos] = tensao;
      validos++;
      }
    }
    return validos;
}

double Medidor::calcularRMS(double *dados, int tamanho) {
    if (tamanho == 0) return 0;
    double soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += dados[i] * dados[i];
    }
    return sqrt(soma / tamanho);
}

double Medidor::adcParaTensao(int leitura) {
    return leitura * 0.000125;
}

void Medidor::obterLeituras(double *buffer, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        int16_t raw;
        raw = ads.readADC_SingleEnded(0);
        buffer[i] = adcParaTensao(raw);
    }
}

double Medidor::medirCorrenteBruta() {
    double *leituras = (double *)malloc(AMOSTRAS * sizeof(double));
    double *filtrados = (double *)malloc(AMOSTRAS * sizeof(double));

    if (!leituras || !leituras) {
        Serial.println("Medidor: erro de alocação de memória.");
        if (leituras) free(leituras);
        if (filtrados) free(filtrados);
        return 0;
    }

    obterLeituras(leituras, AMOSTRAS);
    int validos = filtrarDados(leituras, AMOSTRAS, tensao_offset, filtrados);
    double tensao_RMS = calcularRMS(filtrados, validos);

    free(leituras);
    free(filtrados);

    return (tensao_RMS / resistencia) * voltas_transformador;
}

double Medidor::medirCorrenteCorrigida() {
    double crms = medirCorrenteBruta();
    return (crms - coeficiente_linear) * coeficiente_angular;
}