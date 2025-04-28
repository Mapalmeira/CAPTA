# Configuração do Sistema de Envio de Dados

Este diretório contém o firmware embarcado no ESP32, responsável por coletar, armazenar e enviar medições de corrente elétrica para um Web App no Google Drive, de forma automatizada e segura.

O sistema é dividido em três camadas principais:

- **Medição elétrica**: coleta de corrente usando sensor SCT-013-000.
- **Gerenciamento de envio**: organização de medições e envio HTTPS POST.
- **Gerenciamento de operação**: controle de modos de funcionamento, logs e interface física.

---

## Visão Geral do Funcionamento

### Medição

O ESP32 lê a corrente elétrica via sensor SCT-013-000, utilizando o módulo `Medidor`. As leituras são processadas (cálculo de RMS) e armazenadas em buffer para posterior envio.

### Gerenciamento Inteligente

O sistema adapta sua operação automaticamente:

- Buffer de medições para reenvio em caso de falha.
- Reconexão automática ao Wi-Fi.
- Reinicialização preventiva em caso de erros críticos.
- Modos de operação configuráveis:
  - Envio
  - Pendência
  - Desconexão
  - Desligamento
  - Reinicialização

### Interface Física

Um botão físico permite interações simples (como enviar um comando de desligamento), e um LED integrado indica o modo atual de operação.

- LED piscando a 1 Hz → Envio normal.
- LED piscando 0.5 Hz → Problema de conexão detectado.
- LED piscando a 2 Hz → Envio em pendência.

---

## Passos de instalação

### Configuração do Sistema

Antes de utilizar o sistema, configure o arquivo `envio_esp32.ino`, localizado em `envio/`:

- **Credenciais Wi-Fi**: defina `SSID` e `PASSWORD`.
- **URL do Web App**: configure `ENDERECO_SHEETS`.
- **Token de autenticação**: ajuste o valor de `TOKEN`.

Também é possível configurar:

- Reinicialização periódica (`REINICIALIZACAO_PERIODICA`).
- Quantidade de envios por vez (`ENVIOS_POR_VEZ`).
- Atraso adicional entre medições (`DELAY_ADICIONAL`).
- Atraso progressivo em caso de desconexões (`DELAY_PROGRESSIVO`).
- Definição das casas decimais enviadas (`CASAS_DECIMAIS`).
  *(Exemplo: {-1, 2} significa enviar um número inteiro e outro decimal com 2 casas decimais.)*

Para o funcionamento correto do medidor, é necessário definir:

- Resistência do burden resistor (`RESISTENCIA`).
- Número de voltas do transformador de corrente (`VOLTAS_TRANSFORMADOR`).
- Offset de tensão (`TENSAO_OFFSET`).
- Correções lineares (`COEFICIENTE_ANGULAR` e `COEFICIENTE_LINEAR`).

Esses valores podem ser obtidos a partir do procedimento de calibração descrito em [`hardware/`](./hardware/README.md).

### Instalação

1. Adicione o conteúdo de `envio/` a uma pasta "envio_esp32".
2. Abra a pasta "envio_esp32" criada anteriormente como um projeto no Arduino IDE.
3. Configure `envio_esp32.ino` conforme descrito no tópico anterior.
4. Conecte o ESP32 ao computador.
5. Compile e faça o upload para o ESP32.
6. Verifique o Serial Monitor para acompanhar o status do dispositivo.

---

## Observações

- O sistema implementa verificação de conexão com o Web App e reagenda envios em caso de falha.
- A biblioteca WiFiClientSecure é utilizada em modo inseguro (`client.setInsecure()`), sem verificação de certificados, pois o Apps Script do Google Drive usa certificados dinâmicos difíceis de validar em tempo de execução.

---

## Próximos Passos

Acesse os READMEs específicos para detalhes aprofundados:

- [`hardware/`](./hardware/README.md): sensores, módulos, lógica de medição.
- [`envio/`](./envio/README.md): lógica do firmware do ESP32 e envio dos dados.
- [`servidor/`](./servidor/README.md): Apps Script, salvamento no Drive e planilha de visualização.

---

## Referências

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
