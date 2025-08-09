\\ a(n) = Sum_{k=0..n} 2^k * binomial(n+k-1,k) * binomial(2*n-k,n-k).
a(n) = sum(k=0, n, 2^k * binomial(n+k-1,k) * binomial(2*n-k,n-k));
for(n=0, 20, print1(a(n), ", "));



