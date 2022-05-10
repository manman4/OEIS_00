M=30;

a(n) = n!*sum(k=0, n\2, abs(stirling(n-k, k, 1))/(n-k)!); 
for(n=2, M, print1(a(n), ", "));