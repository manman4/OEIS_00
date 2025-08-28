\\ a(n) = (-1)^n * Sum_{k=0..n} (1/14)^(n-2*k) * binomial(-1/2,k) * binomial(k,n-k).
a(n) = (-1)^n * sum(k=0, n, (1/14)^(n-2*k) * binomial(-1/2,k) * binomial(k,n-k));
for(n=0, 20, print1(a(n),", "))  
