M=20;

a(n) = sum(k=0, n\2, 3^k*stirling(n-k, k, 2)); 
for(n=0, M, print1(a(n), ", "));