M=32;

a(n) = sum(k=1, n, binomial(n, n\k));
for(n=0, M, print1(a(n), ", "));