\\ a(n) = Sum_{k=0..n} 3^k * 4^(n-k) * binomial(n+2,k+2) * Stirling2(k+2,2).
a(n) = sum(k=0, n, 3^k * 4^(n-k) * binomial(n+2,k+2) * stirling(k+2,2,2));
for(n=0, 20, print1(a(n),", "));

\\ a(n) = Sum_{k=0..n} (-3)^k * 10^(n-k) * binomial(n+2,k+2) * Stirling2(k+2,2). 
b(n) = sum(k=0, n, (-3)^k * 10^(n-k) * binomial(n+2,k+2) * stirling(k+2,2,2));
for(n=0, 100, print1(a(n)-b(n),", "));