M=20;

a(n) = sum(k=0, n, (-3)^k*binomial(n-1, n-k)*binomial(2*k, k));
for(n=0, M, print1(a(n), ", "));