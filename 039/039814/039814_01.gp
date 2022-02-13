M=140;

T(n, k) = sum(j=0, n, stirling(n, j, 1)*stirling(j, k, 1)); 
cnt=1;
for(n=1, M, for(k=1, n, write("/Users/xxx/Desktop/b039814_1.txt", cnt, " ", T(n, k)); cnt++));
