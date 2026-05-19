\\ a(n) = Sum_{k=1..n} binomial(k+1,2) * binomial(n+2*k-1,3*k-1).
a(n) = sum(k=1, n, binomial(k+1,2) * binomial(n+2*k-1,3*k-1));
for(n=1, 30, print1(a(n),", "));