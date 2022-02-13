M=10;

T(n, k) = sum(j=0, n, stirling(n, j, 2)*stirling(j, k, 2)); 
cnt=1;
for(n=1, M, for(k=1, n, write("/Users/xxx/Desktop/b039810.txt", cnt, " ", T(n, k)); cnt++));
