M=20;

a(n) = vecmax(Vec(prod(j=1, n, sum(k=0, j, (-x)^k))));
for(n=0, M, print1(a(n), ", "));