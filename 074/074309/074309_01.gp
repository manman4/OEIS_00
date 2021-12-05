M=11;

a(n) = sum(k=n+1, 2*n, k^k);
for(n=1, M, print1(a(n), ", "));