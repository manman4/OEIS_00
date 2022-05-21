M=18;

a(n) = n*sum(k=1, n-1, (k-1)!*abs(stirling(n-1, k, 1))); 
for(n=0, M, print1(a(n), ", "));