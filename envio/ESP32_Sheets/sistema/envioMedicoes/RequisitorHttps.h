#ifndef REQUISITORHTTPS_H
#define REQUISITORHTTPS_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "sistema/logador/Logador.h"

class RequisitorHttps {
private:
    String endereco;
    String token;
    Logador& logador;

    String realizarPOST(const String& dados, int &httpCode);

public:
    RequisitorHttps(Logador& log, String url, String token);
    bool enviarMedicoes(const String& dados);
    bool verificarPOST();
};

#endif
