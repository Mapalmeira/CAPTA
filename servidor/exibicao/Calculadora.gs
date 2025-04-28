class Calculadora {
  static integral(x, y) {
    if (x.length !== y.length || x.length < 2) {
      return 0;
    }

    let areaTotal = 0;
    for (let i = 0; i < x.length - 1; i++) {
      const deltaX = x[i + 1] - x[i];
      const areaTrapezio = ((y[i] + y[i + 1]) / 2) * deltaX;
      areaTotal += areaTrapezio;
    }

    return areaTotal;
  }

  static media(dados) {
    if (dados.length === 0) {
      return 0;
    }

    const soma = dados.reduce((acc, linha) => acc + linha[1], 0);
    return soma / dados.length;
  }
}
