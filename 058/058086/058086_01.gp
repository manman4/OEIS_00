\\ T(n,k) = Sum_{j=0..k} (-1)^(j+k) * j! * binomial(n-j,n-k) * binomial(n+j,2*j).
T(n, k) = sum(j=0, k, (-1)^(j+k) * j! * binomial(n-j, n-k) * binomial(n+j, 2*j));
for(n=0, 9, for(k=0, n, print1(T(n,k),", ")));

