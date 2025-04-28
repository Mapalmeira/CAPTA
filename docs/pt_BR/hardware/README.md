# Hardware de Medição de Corrente Elétrica com ESP32

Este diretório contém os arquivos e instruções para a montagem do sistema de medição de corrente elétrica baseado em um ESP32, incluindo uma explicação da lógica de medição.

---

## Componentes do Circuito do Sensor

O circuito de leitura utiliza o sensor SCT-013-000 com os seguintes componentes:

- 1 × Sensor de corrente SCT-013-000
- 2 × Resistores de 10 kΩ
- 1 × Resistor de 75 Ω (burden resistor)
- 1 × Capacitor de 10 µF

### Observações

O valor de 75 Ω utilizado como burden resistor foi escolhido levando em consideração uma corrente máxima de 32 A, valor típico para disjuntores gerais em residências brasileiras. Essa escolha garante que a tensão de saída do sensor, mesmo em condições de corrente elevada, permaneça dentro da faixa de operação segura do ADC interno do ESP32, que aceita até 3,3 V. Valores de resistência maiores podem ser utilizados sem riscos, embora alterem a sensibilidade da leitura. Já valores menores **não devem ser usados**, pois podem gerar tensões acima do limite suportado pelo microcontrolador, comprometendo a integridade do sistema.

A montagem do circuito segue o esquema proposto pelo projeto OpenEnergyMonitor, uma referência consolidada em medição de energia com microcontroladores.

Imagens ilustrativas e diagramas de ligação estão disponíveis na pasta `imagens/`, sendo a leitura analógica do sinal é feita no pino **36** do ESP32 (identificado como `SP` nos esquemas).

O guia de montagem pode ser consultado em:
[https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html](https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html)

---

## Lógica de Medição

O SCT-013-000 é um transformador de corrente com razão de espiras 2000:1. Ele converte uma corrente de entrada I em uma corrente de saída I dividido por 2000.

Como o ESP32 realiza leituras de **tensão** (entre 0 e 3.3 V), a corrente precisa ser convertida em uma faixa de voltagem compatível. Isso é feito em duas etapas:

1. **Conversão de corrente para tensão:**
   A corrente de saída do sensor passa por um resistor de carga (burden resistor). A tensão gerada oscila entre aproximadamente -I/2000 × √2 × R e +I/2000 × √2 × R, onde R é o valor do resistor em ohms escolhido cautelosamente para manter a amplitude entre -3.3/2 e 3.3/2.

2. **Offset para leitura unipolar:**
   Como o ESP32 não lê tensões negativas, é adicionado um **offset de 1,65 V** ao sinal, para que a tensão fique centrada entre 0 e 3,3 V.

---

## Calibração

Esta seção explica os códigos auxiliares presentes em `hardware/`, essenciais para a calibração do sensor:

### 1. `offset.ino`

Serve para medir o offset medido do sensor desconectado de qualquer carga.

**Passos:**
- Monte o circuito do ESP32+sensor conforme o esquema da pasta.
- Abra o `offset.ino` no Arduino IDE.
- Rode o programa com o sensor desconectado de qualquer carga.
- Leia a saída serial algumas vezes e anote a média da tensão de offset observada.

### 2. `calibracao.ino`

Permite calibrar a medição em amperagem.

**Passos:**
- Abra o código no Arduino IDE.
- Insira o valor do offset obtido no passo anterior na variável `tensao_offset`.
- Altere o valor da variável `resistencia` para refletir precisamente o resistor usado (meça com um multímetro).
- Conecte o sensor a diferentes cargas.
- Compare os valores medidos pelo ESP32 na saída serial com os valores medidos por amperímetro do tipo alicate.

### 3. `correcao_linear.py`

Se os valores obtidos na etapa anterior estiverem discrepantes, será necessário ajustar a medição através de uma correção linear.

Esse código aplica essa correção aos valores medidos para que representem mais fielmente o consumo real.

**Passos:**
- Abra o código em um editor de texto de sua preferência e substitua a lista `dados` por uma lista de pares (valor_medido, valor_esperado)
- Execute o script. Um gráfico será exibido mostrando a regressão linear.
- Os coeficientes `a` e `b` desejados devem aparecer na legenda na forma f(x) = ax + b
- Insira esses coeficientes nos campos abaixo, presentes em `calbricao.ino`:

    double coeficiente_angular = ...; // Inclinação da regressão, o `a` obtido.
    double coeficiente_linear  = ...; // Deslocamento da regressão, o `b` obtido.

- Repita o passo 2 de calibração e confira se a precisão está conforme o esperado.

**Observação**:
Espera-se que o circuito, mesmo sem correção, seja capaz de realizar mensurações boas.
Se os coeficientes obtidos nesta etapa forem muito altos ou caso os pontos do gráfico de regressão pareçam muito espalhados, provavelmente um erro foi cometido nas etapas anteriores à calibração.

---

## Limitações da Medição

Apesar da simplicidade e baixo custo, este tipo de medição apresenta limitações:

### Ausência de voltagem instantânea

O sistema atual mede apenas a **corrente instantânea**. A potência é estimada assumindo uma **tensão constante** (exemplo 110 V ou 220 V). Para uso residencial, essa aproximação costuma ser suficiente, já que a voltagem é relativamente estável e corrente e tensão estão geralmente em fase.

Entretanto, em ambientes industriais (com cargas reativas), essa abordagem pode gerar erros significativos. Nestes casos, recomenda-se adicionar um **módulo de medição de tensão instantânea** ao circuito.

### Precisão do ADC interno

O conversor analógico-digital (ADC) do ESP32 tem limitações de precisão, podendo introduzir erros de até 5%. Embora aceitável para uso geral, o sistema pode ser aprimorado com um ADC externo, como o ADS1115, caso maior precisão seja necessária.

---

## Referência

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)

---

## Próximos Passos

Acesse os READMEs específicos para detalhes aprofundados:

- [`hardware/`](./hardware/README.md): sensores, módulos, lógica de medição.
- [`envio/`](./envio/README.md): lógica do firmware do ESP32 e envio dos dados.
- [`servidor/`](./servidor/README.md): Apps Script, salvamento no Drive e planilha de visualização.

---

## Conteúdo de `hardware/`

- `offset.ino` – Código para medir o offset do circuito.
- `calibracao.ino` – Código para calibrar a medição de corrente.
- `correcao_linear.py` - Código para obter os coeficientes de uma regressão linear.
- `imagens/` – Diagramas de ligação dos módulos ao ESP32.
