#ifndef MEDIDOR_H
#define MEDIDOR_H

#include <WiFi.h>
#include <time.h>
#include <vector>
#include <variant>

using ValorMedicao = std::variant<int, float>;
using Medicao = std::vector<ValorMedicao>;

class Medidor {
    private:
        int PINO_ANALOGICO = 36;
        int TEMPO_COLETA = 1000;
        int AMOSTRAS_POR_SEGUNDO = 750;

        double resistencia = 75
        double voltas_transformador = 2000.0;
        double tensao_offset = 1.65;
        double coeficiente_angular = 1;
        double coeficiente_linear = 0;

        bool ntpInicializado = false;

        void iniciarNTP();
        int getEpoch();

        double calcularMedia(int *dados, int tamanho);
        double calcularDesvioPadrao(int *dados, int tamanho, double media);
        int filtrarDados(int *dados, int tamanho, double offset, double *dados_filtrados);
        double calcularRMS(double *dados, int tamanho);

        double medirCorrenteBruta();
        double medirCorrenteCorrigida();

    public:
        Medidor();

        void setResistencia(double r);
        void setVoltasTransformador(double f);
        void setOffsetTensao(double offset);
        void setCoeficientesCorrecao(double angular, double linear);

        Medicao medir(); // Retorna [timestamp, corrente corrigida]
};

#endif // MEDIDOR_H
