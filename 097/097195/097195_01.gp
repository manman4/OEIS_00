\\ G.f.: Sum_{k in Z} x^k / (1 - x^(6*k + 1)).

v(n)={x='x+O('x^(n+10)); sum(k=-n, n, x^k / (1 - x^(6*k + 1)))};
M=10000;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b097195_01.txt", n, " ", polcoef(v, n)))
