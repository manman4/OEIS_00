M=20;

a388043(n) = sum(k=0, n, binomial(3*n-2*k, n-k));
a390239(n) = sum(k=0, n, binomial(3*n-2*k+2, n-k));

\\ a(n) = A388043(n) - 3*A390239(n-1).
a(n) = a388043(n) - 3*a390239(n-1);
for(n=0, M, print1(a(n),", "));
