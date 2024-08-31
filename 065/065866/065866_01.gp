\\ a(n) = 2 * Sum_{k=0..n} (n+2)^(k-1) * |Stirling1(n,k)|.
a(n) = 2 * sum(k=0, n, (n+2)^(k-1)*abs(stirling(n, k, 1)));
for(n=0, 25, print1(a(n),", "))


