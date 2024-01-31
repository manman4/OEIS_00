bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a004212(n) = 3^n * bell(n, 1/3);
for(n=0, 20, print1(a004212(n), ", "));

\\ a(n) = Sum_{k=0..n} Stirling1(n,k) * A004212(k). 
a(n) = sum(k=0, n, stirling(n, k, 1) * a004212(k));
for(n=0, 20, print1(a(n), ", "));