default(parisize, 120000000);

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); prod(k=1, n, (1-x^(12*k))^2 * (1-x^(12*k-6)) / ((1-x^(12*k-5))*(1-x^(12*k-7))) ) };
M=10000;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b094428_01.txt", n, " ", polcoef(v, n)))