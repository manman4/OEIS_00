\\ \r
\\ Nは使わない 
default(parisize, 120000000);

\\ A068322を利用
v(n)={x='x+O('x^(n+10)); x/(1-x^2)+sum(k=2, sqrt(n), x^(k^2)/((1-x^(2*k))*(1-x^(k*(k-1))))) };
a091954(n) = sumdiv(n, d, (d<n)*(d%2));
M=10000;
v=v(M);
for(n=1, M, write("/Users/xxx/Desktop/b068324.txt", n, " ", polcoef(v, n)+a091954(n)))