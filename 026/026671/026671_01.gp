\\ a(n) = Sum_{k=0..n+1} 4^(n+1-k) * binomial(n-k/2,n+1-k).
a(n) = sum(k=0, n+1, 4^(n+1-k)*binomial(n-k/2, n+1-k));
for(n=0, 40, print1(a(n),", "))
