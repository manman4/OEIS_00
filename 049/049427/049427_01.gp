bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a005011(n) = 5^n * bell(n, 1/5);
for(n=0, 20, print1(a005011(n), ", "));

\\ a(n) = Sum_{k=0..n} Stirling1(n,k) * A005011(k).
a(n) = sum(k=0, n, stirling(n, k, 1) * a005011(k));
for(n=0, 20, print1(a(n), ", "));