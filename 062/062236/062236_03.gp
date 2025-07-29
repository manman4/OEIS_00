\\ a(n) = Sum_{k=0..n-1} binomial(4*k-2+l,k) * binomial(4*n-4*k-l,n-k-1) for every real number l.
a(n, l) = sum(k=0, n-1, binomial(4*k-2+l/3, k)*binomial(4*n-4*k-l/3, n-k-1));
for(l=-20, 20, for(n=0, 10, print1(a(n,0)-a(n,l),", ")); print);
for(n=0, 20, print1(a(n,0),", "));

\\ a(n) = Sum_{k=0..n-1} binomial(3*k-1+l,k) * binomial(3*n-3*k-l,n-k-1) for every real number l.
a(n, l) = sum(k=0, n-1, binomial(3*k-1+l/3, k)*binomial(3*n-3*k-l/3, n-k-1));
for(l=-20, 20, for(n=0, 10, print1(a(n,0)-a(n,l),", ")); print);
for(n=0, 20, print1(a(n,0),", "));

\\ a(n) = Sum_{k=0..n-1} 2^(n-k-1) * binomial(3*n,k).
b(n) = sum(k=0, n-1, 2^(n-k-1)*binomial(3*n, k));
for(n=0, 50, print1(a(n,0)-b(n),", "));

\\ a(n) = Sum_{k=0..n-1} 3^(n-k-1) * binomial(2*n+k,k). 
c(n) = sum(k=0, n-1, 3^(n-k-1)*binomial(2*n+k, k));
for(n=0, 50, print1(a(n,0)-c(n),", "));