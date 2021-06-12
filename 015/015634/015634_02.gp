M=40;

a(n) = binomial(n+3, 4)-sum(k=2, n, a(n\k)); 
for(n=1, M, print1(a(n), ", "));