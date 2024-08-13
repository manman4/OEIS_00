\\ a(n) = (-1)^n * Sum_{k=0..n} binomial(2*k-1,n-k). 
a(n) = (-1)^n * sum(k=0, n, binomial(2*k-1, n-k));
for(n=0, 20, print1(a(n), ", "));
