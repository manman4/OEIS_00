M=200;

my(x='x+O('x^(M+10))); v=serlaplace(1/prod(k=1, M, (1 - x^(2*k-1))^(moebius(2*k-1)/(2*k-1))));
for(n=0, M, write("/Users/xxx/Desktop/b005388_1.txt", n, " ", polcoef(v, n)))
