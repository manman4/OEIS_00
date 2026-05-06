\\ a(n) = Sum_{k=0..n} 6^(n-k)*binomial(7*n+1,k).
a(n) = sum(k=0, n, 6^(n-k) * binomial(7*n+1,k));
for(n=0, 30, print1(a(n), ", "));

\\ a(n) = Sum_{k=0..n} 7^(n-k)*binomial(6*n+k,k).
b(n) = sum(k=0, n, 7^(n-k) * binomial(6*n+k,k));
for(n=0, 30, print1(a(n)-b(n), ", "));



\\ a(0) = 1; a(n) = (14/n) * Sum_{k=0..n-1} 6^k * binomial(k+2,2) * binomial(7*n+1,n-1-k).
c(n) = if(n==0, 1, (14/n) * sum(k=0, n-1, 6^k * binomial(k+2,2) * binomial(7*n+1,n-1-k)));
for(n=0, 30, print1(a(n)-c(n), ", "));

\\ a(0) = 1; a(n) = (14/n) * Sum_{k=0..n-1} 7^k * binomial(k+2,2) * binomial(7*n-2-k,n-1-k). 
d(n) = if(n==0, 1, (14/n) * sum(k=0, n-1, 7^k * binomial(k+2,2) * binomial(7*n-2-k,n-1-k)));
for(n=0, 30, print1(a(n)-d(n), ", "));