\\ a(n) = Sum_{k=0..n} (5*n+1)^(k-1) * |Stirling1(n,k)|. 
a(n) = sum(k=0, n, (5*n+1)^(k-1)*abs(stirling(n, k, 1)));
for(n=0, 25, print1(a(n),", "))


