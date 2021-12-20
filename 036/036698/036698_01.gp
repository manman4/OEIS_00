\\ \r
\\ Nは使わない 

a(n) = sum(k=1, n, (-1)^(k+1)*(n\(2*k-1))); 
M=1000;
for(n=0, M, write("/Users/xxx/Desktop/b036698_1.txt", n, " ", a(n * n)))