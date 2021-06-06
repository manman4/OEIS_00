\\ \r

M=51174625;
a(n)=denominator(sigma(n+1, 1)/sigma(n, 1));
b(n)=sigma(n+1, 1)/sigma(n, 1);
cnt = 1;
for(n=1, M, if(a(n)==1, write("/Users/xxx/Desktop/b077089.txt", cnt, " ", b(n)); cnt++))