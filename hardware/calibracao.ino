int PINO_ANALOGICO = 36;
int TEMPO_COLETA = 1000;              // em milissegundos
int AMOSTRAS_POR_SEGUNDO = 750;

double resistencia = 75.0;            // resistência (ohms) do burden resistor
double voltas_transformador = 2000.0; // razão do transformador de corrente (ex: SCT-013-000)
double tensao_offset = 1.535722;      // valor de offset em volts (medido com offset.ino)
double coeficiente_angular = 1.0;     // coeficiente de escala (1.0 padrão)
double coeficiente_linear = 0.0;      // correção de deslocamento (0.0 padrão)

// Converte valor bruto do ADC para tensão
double adcParaTensao(int leitura) {
  return (leitura / 4095.0) * 3.3;
}

// Calcula a média dos dados
double calcularMedia(int *dados, int tamanho) {
  double soma = 0;
  for (int i = 0; i < tamanho; i++) {
    soma += dados[i];
  }
  return soma / tamanho;
}

// Calcula o desvio padrão dos dados
double calcularDesvioPadrao(int *dados, int tamanho, double media) {
  double soma = 0;
  for (int i = 0; i < tamanho; i++) {
    soma += pow(dados[i] - media, 2);
  }
  return sqrt(soma / tamanho);
}

// Filtra os dados removendo outliers e subtrai o offset
int filtrarDados(int *dados, int tamanho, double offset, double *filtrados) {
  double media = calcularMedia(dados, tamanho);
  double desvio = calcularDesvioPadrao(dados, tamanho, media);

  int validos = 0;
  for (int i = 0; i < tamanho; i++) {
    if (abs(dados[i] - media) < 2 * desvio) {
      double tensao = adcParaTensao(dados[i]) - offset;
      filtrados[validos] = tensao;
      validos++;
    }
  }
  return validos;
}

// Calcula valor RMS de um vetor de tensões
double calcularRMS(double *dados, int tamanho) {
  if (tamanho == 0) return 0;

  double soma_quadrados = 0;
  for (int i = 0; i < tamanho; i++) {
    soma_quadrados += dados[i] * dados[i];
  }
  return sqrt(soma_quadrados / tamanho);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  int num_amostras = (TEMPO_COLETA * AMOSTRAS_POR_SEGUNDO) / 1000;
  int *leituras = (int *)malloc(num_amostras * sizeof(int));
  double *filtrados = (double *)malloc(num_amostras * sizeof(double));

  if (leituras == NULL || filtrados == NULL) {
    Serial.println("Erro ao alocar memória.");
    return;
  }

  // Coleta de amostras
  unsigned long tempo_inicial = micros();
  for (int i = 0; i < num_amostras; i++) {
    leituras[i] = analogRead(PINO_ANALOGICO);
    unsigned long proximo_tempo = tempo_inicial + ((i + 1) * (1000000 / AMOSTRAS_POR_SEGUNDO));
    while (micros() < proximo_tempo) {}
  }

  // Filtra dados e calcula tensão RMS
  int validos = filtrarDados(leituras, num_amostras, tensao_offset, filtrados);
  double tensao_RMS = calcularRMS(filtrados, validos);

  // Libera memória
  free(leituras);
  free(filtrados);

  // Converte tensão RMS para corrente RMS
  double corrente_RMS = (tensao_RMS / resistencia) * voltas_transformador;

  // Aplica correção linear
  corrente_RMS = (corrente_RMS - coeficiente_linear) * coeficiente_angular;

  // Exibe resultado
  Serial.println(corrente_RMS, 5);
}
