a009235(n) = sum(k=0, n, (-2)^(n-k)*stirling(n, k, 2));

\\ a(n) = Sum_{k=0..n} (-4)^(n-k) * Stirling1(n,k) * A009235(k)
a(n) = sum(k=0, n, (-4)^(n-k) * stirling(n, k, 1) * a009235(k));
for(n=0, 20, print1(a(n), ", "));

bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));

\\ (-4)^n * Sum_{k=0..n} (1/2)^k * Stirling1(n,k) * Bell_k(-1/2)
b(n) = (-4)^n * sum(k=0, n, 1/2^k * stirling(n, k, 1) * bell(k, -1/2));
for(n=0, 50, print1(a(n)-b(n), ", "));
