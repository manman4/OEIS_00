\\ a(n) = Sum_{k=0..floor(n/9)} binomial(k,n-9*k). 
a(n) = sum(k=0, n\9, binomial(k, n-9*k));
for(n=0, 85, print1(a(n),", "))


