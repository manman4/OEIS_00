\\ a(n) = Sum_{k=0..n} binomial(3*k+3+l,k) * binomial(3*n-3*k-l,n-k) for every real number l.
a(n, l) = sum(k=0, n, binomial(3*k+3+l/3, k)*binomial(3*n-3*k-l/3, n-k));
for(l=-20, 20, for(n=0, 10, print1(a(n,0)-a(n,l),", ")); print);
for(n=0, 20, print1(a(n,0),", "));

\\ a(n) = Sum_{k=0..n} 2^(n-k) * binomial(3*n+4,k).
b(n) = sum(k=0, n, 2^(n-k)*binomial(3*n+4, k));
for(n=0, 50, print1(a(n,0)-b(n),", "));

\\ a(n) = Sum_{k=0..n} 3^(n-k) * binomial(2*n+k+3,k). 
c(n) = sum(k=0, n, 3^(n-k)*binomial(2*n+k+3, k));
for(n=0, 50, print1(a(n,0)-c(n),", "));