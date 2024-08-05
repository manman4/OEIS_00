\\ a(n) = (-1)^n * Sum_{k=0..floor(n/6)} binomial(n+5,6*k+5). 
a(n) = (-1)^n*sum(k=0,n\6, binomial(n+5,6*k+5));
for(n=0, 25, print1(a(n),", "))

  