from __future__ import division, print_function
 
import numpy as np
import scipy.linalg

import matplotlib.pyplot as pp

l = .25 # rod length is 2l
m = 0.028 # rod 6 mm diameter, 44cm length, 7856 kg/m^3
M = .25
dt = 0.005
g = 9.81


def dlqr(A,B,Q,R):
    """
        Solve the discrete time lqr controller.
        x[k+1] = A x[k] + B u[k]
        cost = sum x[k].T*Q*x[k] + u[k].T*R*u[k]
    """
    #ref Bertsekas, p.151 
    #first, try to solve the ricatti equation
    P = np.matrix(scipy.linalg.solve_discrete_are(A, B, Q, R))     
    #compute the LQR gain
    K = np.matrix(scipy.linalg.inv(B.T*P*B+R)*(B.T*P*A))
    return -K

def getControlGains(dt):    
    A = np.matrix([
        [1, dt, 0, 0], 
        [0, 1, -(3*m*g*dt)/(7*M+4*m), 0],
        [0, 0, 1, dt],
        [0, 0, (3*g*(m+M)*dt)/(l*(7*M+4*m)), 1]
        ])
    B = np.matrix([
        [0, 7*dt/(7*M+4*m), 0, -3*dt/(l*(7*M+4*m))]
        ]).T
    Q = np.matrix([
        [1.0, 0.0, 0.0, 0.0],
        [0.0, .0001, 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, .0001],
        ])
    R = np.matrix([0.01])
    return dlqr(A, B, Q, R)    

K = getControlGains(dt)

print(K)

# times = np.arange(0, 4, dt)
# xs = np.zeros(len(times))
# vs = np.zeros(len(times))
# us = np.zeros(len(times))
# xs[0] = 0.065

# a, b, c = 35.98, 2.22, -2.79

# for i in range(len(times) - 1):
#     x, v = xs[i], vs[i]
#     u = - np.matrix([[x, v]]) * K.T
#     voltage = (u + a * v - c * np.sign(v)) / b
#     dx = v
#     dv = -a * v + b * voltage + c * np.sign(v)
#     xs[i + 1] = x + dx * dt
#     vs[i + 1] = v + dv * dt
#     us[i + 1] = voltage

# print("Control: ", getControlGains(5.0/1000))

# pp.subplot(211)

# pp.plot(times, xs, label="x")
# pp.plot(times, vs, label="v")
# pp.legend()
# pp.grid(True)

# pp.subplot(212)
# pp.plot(times, us, label="u")
# pp.legend()
# pp.grid(True)
# pp.show()

