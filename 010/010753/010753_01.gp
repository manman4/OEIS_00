M=30;

a(n) = sum(k=0, (n-1)\3, binomial(n-k, k)); 
for(n=0, M, print1(a(n), ", "));