M=20;

a(n) = prod(k=1, 5, binomial(n+k+4, n-k+5))/(140*5!);
for(n=0, M, print1(a(n), ", "))