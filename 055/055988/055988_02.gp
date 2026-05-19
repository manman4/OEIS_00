\\ a(n) = Sum_{k=0..n-1} binomial(n-1+3*k, 4*k). 
a(n) = sum(k=0, n-1, binomial(n-1+3*k, 4*k));
for(n=1, 25, print1(a(n),", "));