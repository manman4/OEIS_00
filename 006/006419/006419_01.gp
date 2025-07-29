\\ a(n) = Sum_{k=0..n-1} binomial(2*k+1+l,k) * binomial(2*n-2*k-l,n-k-1) for every real number l.
a(n, l) = sum(k=0, n-1, binomial(2*k+1+l/3, k)*binomial(2*n-2*k-l/3, n-k-1));
for(l=-20, 20, for(n=0, 10, print1(a(n,0)-a(n,l),", ")); print);  
for(n=0, 20, print1(a(n,0),", "));

\\ a(n) = Sum_{k=0..n-1} 2^(n-k-1) * binomial(n+k+2,k). 
b(n) = sum(k=0, n-1, 2^(n-k-1)*binomial(n+k+2, k));
for(n=0, 50, print1(a(n,0)-b(n),", "));