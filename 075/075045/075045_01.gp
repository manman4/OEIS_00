\\ a(n) = Sum_{k=0..n} binomial(3*k+3,k) * binomial(3*n-3*k,n-k) 
a(n) = sum(k=0, n, binomial(3*k+3, k) * binomial(3*n-3*k, n-k));
for(n=0, 20, print1(a(n),", ")); 

\\ Sum_{k=0..n} binomial(3*k,k) * binomial(3*n-3*k+3,n-k).
b(n) = sum(k=0, n, binomial(3*k, k) * binomial(3*n-3*k+3, n-k));
for(n=0, 50, print1(a(n) - b(n),", "));
