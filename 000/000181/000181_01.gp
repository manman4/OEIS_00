\\ T(n, k) = Sum_{j=0..k} (-1)^j*(2*n*(k-j)!/(n+k-j))*binomial(n-k+j, n-k)*binomial(n+k-j, n-k+j), with T(0, k) = 1.
T(n, k) = if(n==0, 1, sum(j=0, k, (-1)^j * (2*n*(k-j)!/(n+k-j)) * binomial(n-k+j, n-k) * binomial(n+k-j, n-k+j)));

\\ a(n) = A058087(n,n-4).
for(n=4, 20, print1(T(n, n-4), ", "));
