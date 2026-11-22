import numpy as np
import matplotlib.pyplot as plt

data=np.loadtxt('motor_output.csv', delimiter=',', skiprows=1)

time=data[:,0]
speed_rad=data[:,1]
speed_rpm=speed_rad*60/(2*np.pi)
ia=data[:,2]
ib=data[:,3]
ic=data[:,4]

fig,ax=plt.subplots(2,1)

ax[0].plot(time,speed_rpm)
ax[0].set_ylabel('Speed [RPM]')
ax[0].set_xlabel('Time [s]')

ax[1].plot(time,ia, label='Ia')
ax[1].plot(time,ib, label='Ib')
ax[1].plot(time,ic, label='Ic')
ax[1].set_ylabel('Current [A]')
ax[1].set_xlabel('Time [s]')
ax[1].legend()

plt.suptitle('PMSM Motor Simulation Results')
plt.tight_layout()
plt.show()
