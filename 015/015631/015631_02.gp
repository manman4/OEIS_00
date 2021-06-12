M=44;

a(n) = binomial(n+2, 3)-sum(k=2, n, a(n\k)); 
for(n=1, M, print1(a(n), ", "));