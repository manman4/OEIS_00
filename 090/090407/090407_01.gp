\\ a(n) = Sum_{k=0..n} binomial(4*n+1,4*k+1).
a(n) = sum(k=0, n, binomial(4*n+1, 4*k+1));
for(n=0, 20, print1(a(n),", "));

\\ a(n) = (1/2) * Sum_{k=0..n} binomial(4*n+2,4*k+1).
b(n) = (1/2) * sum(k=0, n, binomial(4*n+2, 4*k+1));
for(n=0, 50, print1(a(n)-b(n),", "));