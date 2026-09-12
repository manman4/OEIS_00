M=20;

\\ a(n) = C(6*n+1,n).
a(n) = binomial(6*n+1, n);
for(n=0, M, print1(a(n),", "));
