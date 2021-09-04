M=59;

a(n) = sum(i=0, n, sum(j=i, n-i, i*j<=n)); 
for(n=0, M, print1(a(n), ", "));