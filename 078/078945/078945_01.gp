\\ a(0) = 1; a(n) = a(n-1) + 4 * Sum_{k=1..n} binomial(n-1,k-1) * a(n-k).
a(n) = if(n==0, 1, a(n-1) + 4*sum(k=1, n, binomial(n-1,k-1) * a(n-k)));
for(n=0, 15, print1(a(n),", "))      