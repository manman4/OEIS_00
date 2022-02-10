M=30;

a(n) = sum(k=(n+2)\4, n\2, binomial(n-k, k)); 
for(n=0, M, print1(a(n), ", "));