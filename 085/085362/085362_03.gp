M=25;

\\ a(n) = Sum_{k=0..n} (-1)^k * 5^(n-k) * binomial(2*k,k)/(1-2*k) * binomial(n-1,n-k).
a(n) = sum(k=0, n, (-1)^k * 5^(n-k) * binomial(2*k,k)/(1-2*k) * binomial(n-1,n-k));
for(n=0, M, print1(a(n), ", "));