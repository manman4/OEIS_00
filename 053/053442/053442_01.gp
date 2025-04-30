v(n)={x='x+O('x^(n+10)); 1/sqrt(1-4*x^2-2*x^3+x^6) };
M=2917;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b053442.txt", n, " ", polcoef(v, n)))
