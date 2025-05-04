\\ a(n) = Sum_{k=0..n} 3^k * 2^(n-k) * binomial(n+3,k+3) * Stirling2(k+3,3).
a(n) = sum(k=0, n, 3^k * 2^(n-k) * binomial(n+3,k+3) * stirling(k+3,3,2));
for(n=0, 20, print1(a(n),", "));

\\ a(n) = Sum_{k=0..n} (-3)^k * 11^(n-k) * binomial(n+3,k+3) * Stirling2(k+3,3).
b(n) = sum(k=0, n, (-3)^k * 11^(n-k) * binomial(n+3,k+3) * stirling(k+3,3,2));
for(n=0, 100, print1(a(n)-b(n),", "));