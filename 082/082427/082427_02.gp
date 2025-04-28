\\ a(n) = 11*n/2 * n! - 2 * Sum_{k=0..n} (k+1)! * binomial(n,k) for n > 1.
a(n) = 11*n/2 * n! - 2 * sum(k=0, n, (k+1)! * binomial(n,k));
for(n=1, 20, print1(a(n),", "))

