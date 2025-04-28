#ifndef ENVIOMEDICOES_H
#define ENVIOMEDICOES_H

#include <deque>
#include <variant>
#include "RequisitorHttps.h"
#include "sistema/logador/Logador.h"

using ValorMedicao = std::variant<int, float>;
using Medicao = std::vector<ValorMedicao>;      // Vetor de valores mistos (int e float). É o conjunto de medições enviadas pelo Medidor. 
using CasasMedicao = std::vector<int>;          // É a quantidade de casas decimais para cada valor de Medicao. -1 representa valores inteiros.

class EnvioMedicoes {
private:
    std::deque<Medicao> buffer; // Buffer de medições pendentes
    std::function<Medicao(void)> funcaoMedicao; // Função de medição.
    RequisitorHttps requisitor;
    CasasMedicao casasDecimais;
    Logador& logador;

    String formatarMedicao(const Medicao& m);
    String montarPacoteLinhas(int linhas);
    
public:
    EnvioMedicoes(String url, Logador& log, String token,
              std::function<Medicao(void)> funcaoMedicao,
              std::vector<int> casasDecimais);

    // Medição
    bool realizarMedicao();

    // Envio
    String enviarLinhasPOST(int linhas);
    bool statusPOST();
    int getTamanhoBuffer();
};

#endif // ENVIOMEDICOES_H
