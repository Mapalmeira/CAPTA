#include "InterfaceFisica.h"
#include <Arduino.h>

InterfaceFisica::InterfaceFisica() {}

void InterfaceFisica::iniciar(FuncaoGetModo funcGetModo, FuncaoComando funcComando) {
    this->getModo = funcGetModo;
    this->comando = funcComando;

    pinMode(LED_PIN, OUTPUT);
    pinMode(BOTAO_PIN, INPUT_PULLUP);

    xTaskCreatePinnedToCore(
        InterfaceFisica::taskPiscarLED,
        "TaskPiscarLED",
        1024,
        this,
        1,
        nullptr,
        0
    );

    xTaskCreatePinnedToCore(
        InterfaceFisica::taskBotao,
        "TaskBotao",
        1024,
        this,
        1,
        nullptr,
        0
    );
}

void InterfaceFisica::taskPiscarLED(void* parameter) {
    InterfaceFisica* self = static_cast<InterfaceFisica*>(parameter);
    while (true) {
        self->loopPiscarLED();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void InterfaceFisica::taskBotao(void* parameter) {
    InterfaceFisica* self = static_cast<InterfaceFisica*>(parameter);
    while (true) {
        self->loopBotao();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void InterfaceFisica::loopPiscarLED() {
    static int ultimoBlink = 0;
    int tempoAtual = millis();
    int intervalo = 1000;

    String modo = getModo();

    if (modo == "envio") {
        intervalo = 1000;
    } else if (modo == "desconexao") {
        intervalo = estadoLED ? 50 : 2000;
    } else if (modo == "pendencia") {
        intervalo = 500;
    }

    if (tempoAtual - ultimoBlink >= intervalo) {
        ultimoBlink = tempoAtual;
        estadoLED = !estadoLED;
        digitalWrite(LED_PIN, estadoLED);
    }
}

void InterfaceFisica::loopBotao() {
    bool estadoAtual = digitalRead(BOTAO_PIN);

    if (estadoBotaoAnterior == HIGH && estadoAtual == LOW) {
        if (millis() - ultimoPressionado > 500) {
            comando("stop");
            ultimoPressionado = millis();
        }
    }

    estadoBotaoAnterior = estadoAtual;
}
