import numpy as np
import matplotlib.pyplot as pp

def integrate_curve(a, b, c, u, times):
    vs = np.zeros(len(times))
    for i in range(0, len(times) - 1):
        dt = times[i + 1] - times[i]
        dv = -a*vs[i] + b*u + c*np.sign(vs[i])
        vs[i + 1] = vs[i] + dv*dt
    return vs

if __name__ == "__main__":
    dt = 0.01
    times = np.arange(0, 20, dt)
    vs = integrate_curve(3.0, 3.0, 0.1, - 0.1, 12.0, times)
    pp.plot(times, vs)
    pp.xlabel("Time (s)")
    pp.ylabel("Veloxity")
    pp.grid(True)
    pp.show()


