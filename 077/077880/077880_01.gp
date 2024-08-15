\\ a(n) = (-1)^n * Sum_{k=0..n} binomial(k-2,n-k).
a(n) = (-1)^n * sum(k=0, n, binomial(k-2, n-k));
for(n=0, 41, print1(a(n), ", "))