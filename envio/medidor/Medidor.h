#ifndef MEDIDOR_H
#define MEDIDOR_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <time.h>
#include <vector>
#include <variant>

using ValorMedicao = std::variant<int, float>;
using Medicao = std::vector<ValorMedicao>;

class Medidor {
    private:
        Adafruit_ADS1115 ads;

        int AMOSTRAS = 750;

        double resistencia;
        double voltas_transformador;
        double tensao_offset;
        double coeficiente_angular;
        double coeficiente_linear;

        bool ntpInicializado = false;
        bool possuiReferenciaTempo = false;
        time_t epochReferencia = 0;
        uint32_t millisReferencia = 0;
        void iniciarNTP();
        int getEpoch();

        void obterLeituras(double *buffer, int tamanho);
        double adcParaTensao(int leitura);
        double calcularMedia(double *dados, int tamanho);
        double calcularDesvioPadrao(double *dados, int tamanho, double media);
        int filtrarDados(double *dados, int tamanho, double offset, double *dados_filtrados);
        double calcularRMS(double *dados, int tamanho);

        double medirCorrenteBruta();
        double medirCorrenteCorrigida();

    public:
        Medidor();

        void iniciarI2C();
        void setResistencia(double r);
        void setVoltasTransformador(double f);
        void setOffsetTensao(double offset);
        void setCoeficientesCorrecao(double angular, double linear);

        Medicao medir(); // Retorna [timestamp, corrente corrigida]
};

#endif // MEDIDOR_H
