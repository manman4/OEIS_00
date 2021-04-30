M=29;

a(n) = round(prod(k=1, n, 1 + 4*sin(2*k*Pi/(2*n+1))^2));
for(n=0, M, print1(a(n), ", "));