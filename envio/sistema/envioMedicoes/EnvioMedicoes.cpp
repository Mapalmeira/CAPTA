#include "EnvioMedicoes.h"

// Construtor
EnvioMedicoes::EnvioMedicoes(String enderecoAppscript, Logador& log, String token,
                             std::function<Medicao(void)> funcaoMedicao,
                             std::vector<int> casasDecimais)
    : logador(log), requisitor(log, enderecoAppscript, token), funcaoMedicao(funcaoMedicao), casasDecimais(casasDecimais)
{}

// Formata uma única medição para string no formato correto
String EnvioMedicoes::formatarMedicao(const Medicao& m) {
    if (m.size() > casasDecimais.size()) {
        logador.logMensagem("EnvioMedicoes: Quantidade de casas decimais definidas incompatível com a quantidade de medições obtidas.");
        return "";
    }

    String linha = "";
    for (size_t i = 0; i < m.size(); i++) {
        if (i > 0) { linha += "|"; }

        if (casasDecimais[i] == -1) {
            // Se for -1, tratamos como inteiro
            linha += String(std::get<int>(m[i]));
        } else {
            // Se for outro valor, tratamos como float
            linha += String(std::get<float>(m[i]), casasDecimais[i]);
        }
    }
    return linha;
}

// Monta um pacote com múltiplas medições no formato esperado
String EnvioMedicoes::montarPacoteLinhas(int linhas) {
    String pacote = "";
    int linhasEnviadas = 0;

    while (linhasEnviadas < linhas && !buffer.empty()) {
        String medicaoFormatada = formatarMedicao(buffer.at(linhasEnviadas));

        if (linhasEnviadas > 0) { pacote += "_"; }

        pacote += medicaoFormatada;
        linhasEnviadas++;
    }

    if (linhasEnviadas == 0) {
        logador.logMensagem("EnvioMedicoes: Nenhuma medição válida para envio.");
        return "";
    }

    return pacote;
}

// Realiza e adiciona uma medição ao buffer
bool EnvioMedicoes::realizarMedicao() {
    if (funcaoMedicao == nullptr) {
        logador.logMensagem("EnvioMedicoes: Nenhuma função de medição foi definida!");
        return false;
    }

    Serial.println("EnvioMedicoes: realizando a medição...");
    Medicao dados = funcaoMedicao();

    if (dados.empty()) {
        logador.logMensagem("EnvioMedicoes: Função de medição retornou dados vazios!");
        return false;
    }

    buffer.push_back(dados);
    return true;
}

// Envia linhas por requisição POST
String EnvioMedicoes::enviarLinhasPOST(int linhas) {
    if (buffer.empty()) {
        logador.logMensagem("EnvioMedicoes: Nenhuma medição disponível para envio.");
        return "buffer vazio";
    }

    if (linhas > buffer.size()) {
        linhas = buffer.size();
    }

    String pacote = montarPacoteLinhas(linhas);

    if (pacote.isEmpty()) {
        logador.logMensagem("EnvioMedicoes: Pacote de envio vazio. Cancelando envio.");
        return "erro medicao";
    }

    bool enviado = requisitor.enviarMedicoes(pacote);

    if (enviado) {
        Serial.println("EnvioMedicoes: Pacote enviado.");
        buffer.erase(buffer.begin(), buffer.begin() + linhas);
        return "enviado";
    } else {
        logador.logMensagem("EnvioMedicoes: Falha ao enviar medições para o Drive.");
        return "erro conexao";
    }
}

bool EnvioMedicoes::statusPOST() {
    return requisitor.verificarPOST();
}

int EnvioMedicoes::getTamanhoBuffer() {
    return buffer.size();
}
