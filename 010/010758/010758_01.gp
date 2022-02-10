M=30;

a(n) = sum(k=n\4+1, n\2, binomial(n-k, k)); 
for(n=0, M, print1(a(n), ", "));