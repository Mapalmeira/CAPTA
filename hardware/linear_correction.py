import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

# Example list of (measured, expected) pairs
data = [
    (1.1, 2),
    (2.0, 4),
    (3.2, 6),
    (4.1, 8),
    (5.0, 10),
]

# Split the data.
measured = np.array([row[0] for row in data]).reshape(-1, 1)
expected = np.array([row[1] for row in data])

# Create and fit the regression model.
model = LinearRegression()
model.fit(measured, expected)

# Get the coefficients.
a = model.coef_[0]
b = model.intercept_

print(f"Calculated function: f(x) = {a:.3f}x + {b:.3f}")

# Generate points for the regression line.
x_line = np.linspace(min(measured), max(measured), 100).reshape(-1, 1)
y_line = model.predict(x_line)

# Plot the data and regression line.
plt.scatter(measured, expected, color='blue', label='Original data')
plt.plot(x_line, y_line, color='red', label=f'f(x) = {a:.3f}x + {b:.3f}')
plt.xlabel('Measured')
plt.ylabel('Expected')
plt.title('Linear Regression')
plt.legend()
plt.grid(True)
plt.show()
