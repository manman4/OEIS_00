\\ a(n) = 2^(3*n-1) - 3 * Sum_{k=0..n-2} binomial(3*n-1,k) for n > 0.
a(n) = if(n==0, 1, 2^(3*n-1) - 3 * sum(k=0, n-2, binomial(3*n-1, k)));
for(n=0, 15, print1(a(n), ", "));