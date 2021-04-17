M=30;

a(n) = sumdiv(n, d, eulerphi(n/d)*2^(d-1));
for(n=1, M, print1(a(n), ", "));