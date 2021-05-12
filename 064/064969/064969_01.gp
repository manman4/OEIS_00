M=48;

a160889(n) = sumdiv(n, d, moebius(n/d)*d^3)/eulerphi(n);
a(n) = sumdiv(n, d, a160889(d));
for(n=1, M, print1(a(n), ", "));