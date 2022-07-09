M=30;

a(n) = n!*sum(k=0, n\3, stirling(n-2*k, k, 1)/(n-2*k)!); 
for(n=0, M, print1(a(n), ", "));