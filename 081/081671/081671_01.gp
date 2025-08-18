\\ a(n) = (1/2)^n * Sum_{k=0..n} 3^k * binomial(2*k,k) * binomial(2*(n-k),n-k).
a(n) = (1/2)^n * sum(k=0, n, 3^k * binomial(2*k,k) * binomial(2*(n-k),n-k));
for(n=0, 15, print1(a(n),", "));
