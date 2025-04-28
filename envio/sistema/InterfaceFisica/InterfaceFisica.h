#ifndef INTERFACEFISICA_H
#define INTERFACEFISICA_H

class InterfaceFisica {
    public:
        using FuncaoGetModo = std::function<String(void)>;
        using FuncaoComando = std::function<void(String)>;

        InterfaceFisica();

        void iniciar(FuncaoGetModo getModo, FuncaoComando comando);

    private:
        FuncaoGetModo getModo;
        FuncaoComando comando;

        void loopPiscarLED();
        void loopBotao();

        static void taskPiscarLED(void* parameter);
        static void taskBotao(void* parameter);

        bool estadoLED = false;
        bool estadoBotaoAnterior = HIGH;
        unsigned long ultimoPressionado = 0;

        static const int LED_PIN = 2;
        static const int BOTAO_PIN = 0;
};

#endif
