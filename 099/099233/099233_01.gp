M=139;

T(n, k) = sum(j=0, n, binomial(k*(n-j), j));
cnt=0;
for(n=0, M, for(k=0, n, write("/Users/xxx/Desktop/b099233_01.txt", cnt, " ", T(k, n-k)); cnt++))
