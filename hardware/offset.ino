int PINO_ANALOGICO = 36;
int AMOSTRAS = 10000;
int AMOSTRAS_POR_SEGUNDO = 750;

// Converte valor ADC para tensão (0 a 3.3V)
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

// Calcula a média filtrada (remove outliers)
double calcularOffsetTensao(int *dados, int tamanho) {
  double media = calcularMedia(dados, tamanho);
  double desvio = calcularDesvioPadrao(dados, tamanho, media);

  double soma = 0;
  int validas = 0;
  for (int i = 0; i < tamanho; i++) {
    if (abs(dados[i] - media) < 2 * desvio) {
      soma += adcParaTensao(dados[i]);
      validas++;
    }
  }

  if (validas == 0) return 0;
  return soma / validas;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda inicialização do Serial

  int *leituras = (int *)malloc(AMOSTRAS * sizeof(int));
  if (leituras == NULL) {
    Serial.println("Erro ao alocar memória.");
    return;
  }

  for (int i = 0; i < AMOSTRAS; i++) {
    leituras[i] = analogRead(PINO_ANALOGICO);
    delayMicroseconds(1000000 / AMOSTRAS_POR_SEGUNDO);
  }

  double offset = calcularOffsetTensao(leituras, AMOSTRAS);
  Serial.print("Offset de tensão (V): ");
  Serial.println(offset, 6);

  free(leituras);
}

void loop() {
}
