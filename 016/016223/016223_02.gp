\\ a(n) = Sum_{k=0..n} 3^k * binomial(n+2,k+2) * Stirling2(k+2,2).
a(n) = sum(k=0, n, 3^k*binomial(n+2, k+2)*stirling(k+2, 2, 2));
for(n=0, 35, print1(a(n),", "))  