\\ a(n) = (-1)^n * Sum_{k=0..floor(n/2)} binomial(k-1,n-2*k). 
a(n) = (-1)^n * sum(k=0, n\2, binomial(k-1, n-2*k));
for(n=0, 45, print1(a(n),", "))   
