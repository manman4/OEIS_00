\\ a(n) = Sum_{k=0..floor(n/10)} binomial(k,n-10*k).
a(n) = sum(k=0, n\10, binomial(k, n-10*k));
for(n=0, 85, print1(a(n),", "))


