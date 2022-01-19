default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(k=1, n, (2*k-1)*x^(2*k-1)*prod(j=1, k-1, 1+x^(2*j-1)))};
M=1000;
v=v(M);
for(n=1, M, write("/Users/xxx/Desktop/b092316_1.txt", n, " ", polcoef(v, n)))