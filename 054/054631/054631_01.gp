T(n, k) = sumdiv(n, d, eulerphi(d)*k^(n/d))/n;
for(n=1, 10, for(k=1, n, print1(T(n, k), ", ")))
