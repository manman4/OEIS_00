\\ a(n) = Sum_{k=0..floor(n/2)} binomial(4*n+1,n-2*k).
a(n) = sum(k=0, n\2, binomial(4*n+1, n-2*k));
for(n=0, 19, print1(a(n), ", "))