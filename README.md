# CAPTA – Coleta e Armazenamento de Potência com Transmissão Automática

## Monitor de Consumo Elétrico com ESP32 e Google Drive

Este projeto consiste em um sistema completo para medição, envio, armazenamento e visualização de dados de consumo elétrico, utilizando um ESP32 como unidade de medição e o Google Drive como backend de armazenamento e visualização.

O sistema é dividido em três partes principais:

1. **Hardware e medição**: inclui o circuito conectado ao ESP32 e a lógica de medição.
2. **Envio dos dados**: código embarcado no ESP32 responsável pelo envio periódico das medições.
3. **Recebimento e visualização**: scripts hospedados no Google Apps Script que recebem os dados, armazenam em planilhas ou arquivos `.csv` e exibem visualizações interativas.


---

## Componentes Utilizados

- **ESP32**: microcontrolador principal.
- **Sensor SCT-013-000**: transformador de corrente para medição de corrente alternada.
- **Componentes eletrônicos**: um capacitor de 10μF, um resistor de 75 Ω, dois resistores de 10kΩ.
- **Cartão microSD (e módulo externo)** (opcional): armazenamento de dados locais e arquivos de configuração.

---

## Funcionamento Geral

### Medição

O ESP32 realiza leituras da corrente elétrica utilizando o sensor SCT-013-000, seguindo recomendações do projeto OpenEnergyMonitor. A medição é feita com base na média quadrática da corrente (valor RMS), considera um valor fixo de tensão (como 110V ou 220V) e assume que corrente e tensão estão em fase — o que geralmente é válido para residências comuns.

---

### Envio dos Dados

Todos os dados são enviados ao Google Drive exclusivamente via requisições **POST**, direcionadas a um Web App desenvolvido com Google Apps Script.

Pequenas amostras são enviadas periodicamente ao longo do dia. Isso é ideal para visualização em tempo real e acompanhamento contínuo em regiões com acesso a internet.

O sistema conta com:
- Buffer de medições para garantir confiabilidade.
- Lógica de reenvio automático em caso de falha.
- Reinicializações preventivas.

---

### Recebimento

No lado do servidor, o Apps Script trata as requisições POST, armazenando os dados recebidos em planilhas diárias na pasta `dadosPart/`.

### Visualização

Os dados são visualizados por meio de uma planilha do Google Sheets com um script associado. A planilha permite:

- Visualizar a **potência instantânea ao longo de um dia**.
- Ver o **consumo diário (kWh)** dos últimos 30 dias.
- Filtrar intervalos de tempo específicos (ex.: apenas entre 9h e 18h).
- Acionar gráficos por meio de gatilhos simples na planilha.

---

## Observações Importantes

- O sistema atualmente só envia pares `[timestamp, corrente]`, mas o código foi estruturado para permitir fácil expansão para outros tipos de medições.
- A precisão é satisfatória para ambientes residenciais, mas limitações técnicas existem (ver README de hardware).

---

## Próximos Passos

Acesse os READMEs específicos para detalhes aprofundados:

- [`/hardware/`](./hardware/README.md): sensores, módulos, lógica de medição.
- [`/envio/`](./envio/README.md): lógica do firmware do ESP32 e envio dos dados
- [`/servidor/`](./servidor/README.md): Apps Script, salvamento no Drive e planilha de visualização

---

## Referências

- [OpenEnergyMonitor Docs](https://docs.openenergymonitor.org/electricity-monitoring/index.html)

