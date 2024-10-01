\\ a(n) = Sum_{k=0..floor(n/6)} binomial(k,n-6*k). 
a(n) = sum(k=0, n\6, binomial(k, n-6*k));
for(n=0, 55,print1(a(n),", "))


