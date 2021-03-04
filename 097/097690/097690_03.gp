M=20;

a(n) = sum(k=0, n, (n-2)^k*binomial(n+1+k, 2*k+1));
for(n=1, M, print1(a(n), ", "));