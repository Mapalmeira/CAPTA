#ifndef CONEXAO_H
#define CONEXAO_H

#include <WiFi.h>
#include "sistema/logador/Logador.h"

class Conexao {
private:
    String rede_wifi;
    String senha_wifi;

public:
    Conexao(String ssid, String senha);
    bool conectarWIFI();
    bool verificaStatus();
    String exibirStatus();
};

#endif
