\\ a(n) = Sum_{k=0..n} 2^k * (-1)^(n-k) * binomial(4*n,k) * binomial(2*n-k,n-k).
a(n) = sum(k=0, n, 2^k * (-1)^(n-k) * binomial(4*n, k) * binomial(2*n-k, n-k));
for(n=0, 20, print1(a(n), ", "));



