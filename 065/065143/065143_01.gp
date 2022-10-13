M=15;

a(n) = if(n==0, 1, 4*sum(k=0, n-1, binomial(n-1, k)*b(k)));
b(n) = if(n==0, 0,   sum(k=0, n-1, binomial(n-1, k)*a(k)));
for(n=0, M, print1(a(n), ", "));
for(n=0, M, print1(b(n), ", "));