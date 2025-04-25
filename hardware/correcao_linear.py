import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

# Exemplo de lista [(medido, esperado)]
dados = [
    (1.1, 2),
    (2.0, 4),
    (3.2, 6),
    (4.1, 8),
    (5.0, 10),
]

# Separar os dados
medidos = np.array([d[0] for d in dados]).reshape(-1, 1)
esperados = np.array([d[1] for d in dados])

# Criar e ajustar o modelo de regressão
modelo = LinearRegression()
modelo.fit(medidos, esperados)

# Obter os coeficientes
a = modelo.coef_[0]
b = modelo.intercept_

print(f"Função encontrada: f(x) = {a:.3f}x + {b:.3f}")

# Gerar pontos para a linha de regressão
x_linha = np.linspace(min(medidos), max(medidos), 100).reshape(-1, 1)
y_linha = modelo.predict(x_linha)

# Plotar
plt.scatter(medidos, esperados, color='blue', label='Dados originais')
plt.plot(x_linha, y_linha, color='red', label=f'f(x) = {a:.3f}x + {b:.3f}')
plt.xlabel('Medido')
plt.ylabel('Esperado')
plt.title('Regressão Linear')
plt.legend()
plt.grid(True)
plt.show()