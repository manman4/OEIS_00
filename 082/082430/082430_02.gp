\\ a(n) = -19*n/2 * n! + 4 * Sum_{k=0..n} (k+1)! * binomial(n,k) for n > 1.
a(n) = -19*n/2 * n! + 4 * sum(k=0, n, (k+1)! * binomial(n,k));
for(n=1, 20, print1(a(n),", "))

