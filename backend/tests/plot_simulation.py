import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt("C://ws//Motor_Application//backend//motor_output.csv", delimiter=",", skiprows=1)

time = data[:, 0]
current = data[:, 1]
speed = data[:, 2]

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, figsize=(8, 6))

ax1.plot(time, current, color='red', label='Strom (A)')
ax1.set_ylabel('Strom [A]')
ax1.grid(True)
ax1.legend()

ax2.plot(time, speed, color='blue', label='Drehzahl (rad/s)')
ax2.set_xlabel('Zeit [s]')
ax2.set_ylabel('Drehzahl [rad/s]')
ax2.grid(True)
ax2.legend()

plt.tight_layout()
plt.show()