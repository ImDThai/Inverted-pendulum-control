clear all
clc
syms Ucw Uvw Uem U x v theta w kcw ksu kvw kww kem n
r = 0.00612;
M_c = 0.25;
I = 0.00527;
l = 0.26322;
g = 9.8;
b = 0.0;
c = 1.5;
m = 0.07;
M = 0.25;
Lmax = 0.2;
Vmax = 0.5;
Wmax = 10;

kcw = 1.5;
ksu = 1.5;
kvw = 0.75;
kww = 2.5;
kem = 0.5;
n = 1.05;

E = 0.5*(I+m*l^2) + m*g*l*(1-cos(theta));
Er = 2*m*g*l;
Err = E - Er;
Ucw = kcw*sign(x)*log(1-abs(x)/(Lmax)) - ksu*sign(w*cos(theta));
Uvw = kvw*sign(v)*log(1-abs(v)/(Vmax));
Uww = kww*sign(w)*log(1-abs(w)/(Wmax));
Uem = kem*(exp(abs(E - n*Er))-1)*sign(Err)*sign(w*cos(theta));
U = Ucw + Uvw + Uww + Uem;

SwingupControl = (M+m)*U - m*l*w*w*sin(theta) - m*l*cos(theta)*(m*l*U*cos(theta) + m*g*l*sin(theta))/(I+m*l^2);
SwingupControl = vpa(SwingupControl, 2);

AA = I*(M+m) + M*m*(l^2);
aa = (((m*l)^2)*g)/AA;
bb = ((I +m*(l^2))/AA)*c;
cc  = (b*m*l)/AA;
dd  = (m*g*l*(M+m))/AA;
ee  = ((m*l)/AA)*c;
ff  = ((M+m)*b)/AA;
mm = (I +m*(l^2))/AA;
nn = (m*l)/AA;
A  =  [0 0 1 0; 0 0 0 1; 0 aa -bb -cc; 0 dd -ee -ff];
B = [0;0; mm; nn]; 
Q = diag([1 1 0.0001 0.0001]);
R  = 0.01;

eig(A);
rank(ctrb(A,B));

Kr = lqr(A,B,Q,R)

