M=20;

\\ a(n) = -(1/n) * Sum_{d|n} mu(n/d) * (-3)^d.
a(n) = -(1/n) * sumdiv(n, d, moebius(n/d) * (-3)^d);
for(n=1, M, print1(a(n), ", "));