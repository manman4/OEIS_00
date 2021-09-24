M=8;

row(n) = Vec(n!*sum(k=0, n, binomial(x-n, k)));
for(n=0, M, print1(row(n)); print);