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

- LED piscando a 1 hz → Envio normal.
- LED piscando 0.5 hz → Problema de conexão detectado.
- LED piscando a 2 hz → Envio em pendência.

---

## Configurações do Sistema

Antes de utilizar o sistema, configure no arquivo `ESP32_Sheets.ino`:

- **Credenciais Wi-Fi**: `SSID` e `PASSWORD`.
- **URL do Web App**: `ENDERECO_SHEETS`.
- **Token de autenticação**: `TOKEN`.

Também podem ser configurados:

- Se haverá reinicialização periódica (`REINICIALIZACAO_PERIODICA`).
- Quantidade de envios por vez (`ENVIOS_POR_VEZ`).
- Delay adicional entre medições (`DELAY_ADICIONAL`).
- Se haverá progressivo em desconexões (`DELAY_PROGRESSIVO`).
- Definição das casas decimais enviadas (`CASAS_DECIMAIS`).

E, para o medidor:

- Resistência do burden resistor (`RESISTENCIA`).
- Número de voltas do transformador de corrente (`VOLTAS_TRANSFORMADOR`).
- Offset de tensão (`TENSAO_OFFSET`).
- Correções lineares (`COEFICIENTE_ANGULAR` e `COEFICIENTE_LINEAR`).

---

## Observações

- O sistema implementa verificação de conexão com o Web App e reagenda envios em caso de falha.
- A biblioteca WiFiClientSecure é utilizada em modo inseguro (`client.setInsecure()`), sem verificação de certificados, pois o Apps Script do Google Drive usa certificados dinâmicos difíceis de validar em tempo de execução.

---

## Referências

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
