M=1000;
v(M) = my(q='q+O('q^(M+100))); sum(k=1, M, (-1)^(k+1) * q^(k*(k+1)/2) / prod(j=1, k, 1 - q^j) / (1 - q^k)^3);
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b059820_1.txt", n, " ", polcoef(v, n)))

