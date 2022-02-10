M=30;

a(n) = sum(k=0, (n-2)\4, binomial(n-k, k));
for(n=0, M, print1(a(n), ", "));