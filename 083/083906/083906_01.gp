M=48;

a(n) = sum(m=0, n, prod(k=0, m-1, (x^n - x^k) / (x^m - x^k)));
cnt=0;
for(n=0, M, v=a(n); for(k=0, n^2\4, write("/Users/xxx/Desktop/b083906_01.txt", cnt, " ", polcoef(v, k)); cnt++))