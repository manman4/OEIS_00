\\ a(n) = Sum_{k=0..n} 2^k * (-1)^(n-k) * binomial(n,k) * binomial(3*n+k,k).
a(n) = sum(k=0, n, 2^k * (-1)^(n-k) * binomial(n,k) * binomial(3*n+k,k));
for(n=0, 20, print1(a(n),", "));


   