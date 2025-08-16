M=25;

\\ a(n) = Sum_{k=0..n} (-1)^(n-k) * binomial(4*n+1,k).
a(n) = sum(k=0, n, (-1)^(n-k) * binomial(4*n+1,k));
for(n=0, M, print1(a(n)", "));