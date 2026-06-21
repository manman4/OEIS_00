\\ T(n,k) = Sum_{j=0..k} (-1)^(j+k) * j! * binomial(n-j,n-k) * binomial(n+j,2*j).
T(n, k) = sum(j=0, k, (-1)^(j+k) * j! * binomial(n-j, n-k) * binomial(n+j, 2*j));

\\ a(n) = -Sum_{k=3..n} (-1)^k * (n-k)! * binomial(k,3) * binomial(2*n-k,k).
a(n) = -sum(k=3, n, (-1)^k * (n-k)! * binomial(k, 3) * binomial(2*n-k, k));
for(n=0, 20, print1(a(n), ", "));
for(n=0, 50, print1(a(n)-T(n,n-3), ", "));