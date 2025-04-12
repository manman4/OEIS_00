M=11;

\\ A(n,k) = -(1/n) * Sum_{d|n} mu(n/d) * (-k)^d.
a(n, k) = -sumdiv(n, d, moebius(n/d)*(-k)^d)/n;
for(n=1, M, for(k=1, n, print1(a(k, n-k+1), ", ")));  

\\ G.f. of column k: Sum_{j>=1} mu(j) * log(1 + k*x^j) / j.
b(n, k) = polcoef(sum(j=1, n, moebius(j) * log(1 + k*x^j + x*O(x^n)) / j), n);
for(n=1, 20, for(k=1, n, print1(a(k, n-k+1)-b(k, n-k+1), ", ")));  

\\ Product_{n>=1} 1/(1 - x^n)^A(n,k) = 1 + k*x.
c(n, k) = polcoef(prod(m=1, n, 1/(1 - x^m + x*O(x^n))^a(m, k)), n);
d(n, k) = polcoef(1 + k*x, n);
for(n=1, 20, for(k=1, n, print1(c(k, n-k+1)-d(k, n-k+1), ", ")));  