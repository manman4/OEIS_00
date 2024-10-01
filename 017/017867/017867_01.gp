\\ a(n) = Sum_{k=0..floor(n/8)} binomial(k,n-8*k).
a(n) = sum(k=0, n\8, binomial(k, n-8*k));
for(n=0, 85, print1(a(n),", "))


