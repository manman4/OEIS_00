M=64;

a(n) = sum(i=1, n, sum(j=i, n-i, i*j<=n)); 
for(n=0, M, print1(a(n), ", "));