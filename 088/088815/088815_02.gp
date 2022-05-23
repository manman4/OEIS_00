M=30;

a000262(n) = if(n==0, 1, (n-1)!*pollaguerre(n-1, 1, -1));
a(n) = sum(k=0, n, a000262(k)*abs(stirling(n, k)));
for(n=0, M, print1(a(n), ", "));