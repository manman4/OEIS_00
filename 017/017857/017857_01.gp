\\ a(n) = Sum_{k=0..floor(n/7)} binomial(k,n-7*k).
a(n) = sum(k=0, n\7, binomial(k, n-7*k));
for(n=0, 55,print1(a(n),", "))


