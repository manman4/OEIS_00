\\ a(n) = Sum_{k=0..n} 5^(n-k) * binomial(6*n+1,k).
a(n) = sum(k=0, n, 5^(n-k) * binomial(6*n+1,k));
for(n=0, 30, print1(a(n), ", "));

\\ a(n) = Sum_{k=0..n} 6^(n-k) * binomial(5*n+k,k). 
b(n) = sum(k=0, n, 6^(n-k) * binomial(5*n+k,k));
for(n=0, 30, print1(a(n)-b(n), ", "));



\\ a(0) = 1; a(n) = (12/n) * Sum_{k=0..n-1} 5^k * binomial(k+2,2) * binomial(6*n+1,n-1-k).
c(n) = if(n==0, 1, (12/n) * sum(k=0, n-1, 5^k * binomial(k+2,2) * binomial(6*n+1,n-1-k)));
for(n=0, 30, print1(a(n)-c(n), ", "));

\\ a(0) = 1; a(n) = (12/n) * Sum_{k=0..n-1} 6^k * binomial(k+2,2) * binomial(6*n-2-k,n-1-k).
d(n) = if(n==0, 1, (12/n) * sum(k=0, n-1, 6^k * binomial(k+2,2) * binomial(6*n-2-k,n-1-k)));
for(n=0, 30, print1(a(n)-d(n), ", "));