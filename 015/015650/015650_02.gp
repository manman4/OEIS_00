M=36;

a(n) = binomial(n+4, 5)-sum(k=2, n, a(n\k)); 
for(n=1, M, print1(a(n), ", "));