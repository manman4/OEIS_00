M=20;

a(n) = sum(k=0, n, (n-2)^(n-k)*binomial(2*n+1-k, k));
for(n=1, M, print1(a(n), ", "));