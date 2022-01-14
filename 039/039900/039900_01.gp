default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(k=0, n, x^k*(1-x^k^2)/prod(j=1, k, 1-x^j))};
M=1000;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b039900.txt", n, " ", polcoef(v, n)))