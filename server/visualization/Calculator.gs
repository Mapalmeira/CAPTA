class Calculator {
  static integral(x, y) {
    if (x.length !== y.length || x.length < 2) {
      return 0;
    }

    let totalArea = 0;
    for (let i = 0; i < x.length - 1; i++) {
      const deltaX = x[i + 1] - x[i];
      const trapezoidArea = ((y[i] + y[i + 1]) / 2) * deltaX;
      totalArea += trapezoidArea;
    }

    return totalArea;
  }

  static average(data) {
    if (data.length === 0) {
      return 0;
    }

    const sum = data.reduce((accumulator, row) => accumulator + row[1], 0);
    return sum / data.length;
  }
}
