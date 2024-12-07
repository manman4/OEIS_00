M=22;

a(n, r=2, s=2, t=2, u=0) = r*sum(k=0, n, binomial(t*k+u*(n-k)+r, k)*binomial(s*k, n-k)/(t*k+u*(n-k)+r));
for(n=0, M, print1(a(n),", "))                 

\\ a(n) = Sum_{k=0..n} binomial(2*k+2,k) * binomial(2*k,n-k)/(k+1).
b(n) = sum(k=0, n, binomial(2*k+2,k)*binomial(2*k,n-k)/(k+1));
for(n=0, 100, print1(a(n)-b(n),", "))
