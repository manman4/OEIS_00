\\ a(n) = Sum_{k=0..floor(n/4)} (-9)^k * binomial(-1/3,k) * binomial(n-k,n-4*k).
a(n) = sum(k=0, n\4, (-9)^k * binomial(-1/3,k) * binomial(n-k,n-4*k));
for(n=0, 25, print1(a(n),", ")) 
