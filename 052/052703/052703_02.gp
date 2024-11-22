\\ a(n) = 3 * Sum_{k=0..n-3} binomial(n+k,k) * binomial(k,n-3-k)/(n+k).
a(n) = 3 * sum(k=0, n-3, binomial(n+k,k) * binomial(k,n-3-k)/(n+k));
for(n=0, 20, print1(a(n), ", "));
