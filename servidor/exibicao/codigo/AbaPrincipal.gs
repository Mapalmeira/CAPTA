class AbaPrincipal extends AbaPlanilha {
  constructor() {
    super("Principal");
  }

  obterVoltagem() {
    const volts = Number(this.obterValor("C2"));

    if (!volts) {
      throw new Error("Erro: Defina a voltagem.");
    }

    return volts;
  }

  obterDia() {
    const dia = this.obterValor("C3");

    if (!dia || isNaN(dia.getTime())) {
      throw new Error("Erro: Parâmetros de data inválidos.");
    }

    return dia;
  }

  obterHoraInicial() {
    const texto = this.obterValor("C4");
    const horaInicial = parseInt(texto);

    if (isNaN(horaInicial)) {
      throw new Error("Erro: Parâmetros de hora inválidos.");
    }

    return horaInicial;
  }

  obterHoraFinal() {
    const texto = this.obterValor("C5");
    const horaFinal = parseInt(texto);

    if (isNaN(horaFinal)) {
      throw new Error("Erro: Parâmetros de hora inválidos.");
    }

    return horaFinal;
  }
}