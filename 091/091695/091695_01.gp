a(n) = n!*sum(k=0, n, binomial(n+2*k-1, n-k)/k!);  
for(n=0, 20, print1(a(n),", "))