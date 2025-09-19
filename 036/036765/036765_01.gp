M=35;

a001005(n) = sum(k=0, n\2, binomial(k, n-2*k)*binomial(n+1, k))/(n+1);
for(n=0, M, print1(a001005(n),", "))  

\\ a(n) = Sum_{k=0..n} binomial(n,k) * A001005(k).
a(n) = sum(k=0, n, binomial(n, k)*a001005(k));
for(n=0, M, print1(a(n),", "))

