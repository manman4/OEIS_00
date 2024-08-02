bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));


\\ (-4)^n * Sum_{k=0..n} (1/2)^k * Stirling1(n,k) * Bell_k(-1/2)
a(n) = (-4)^n * sum(k=0, n, 1/2^k * stirling(n, k, 1) * bell(k, -1/2));
for(n=0, 20, print1(a(n), ", "));
