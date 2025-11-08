M=24;

\\ a(n) = Sum_{k=0..n} (-1)^k * (2*k+1) * binomial(3*n-k+1,n-k)/(3*n-k+1).
a(n) = sum(k=0, n, (-1)^k * (2*k+1) * binomial(3*n-k+1,n-k)/(3*n-k+1));
for(n=0, M, print1(a(n),", "));  

