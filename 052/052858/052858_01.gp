\\ a(0) = a(1) = 0; a(n) = n + Sum_{k=2..n-1} k * binomial(n-1,k) * a(n-k).
a(n) = if(n<2, 0, n + sum(k=2, n-1, k * binomial(n-1, k) * a(n-k)));
for(n=0, 20, print1(a(n), ", "))
