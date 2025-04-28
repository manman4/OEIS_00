\\ a(n) = -7 * n * n! + 3 * Sum_{k=0..n} (k+1)! * binomial(n,k) for n > 1.
a(n) = -7 * n * n! + 3 * sum(k=0, n, (k+1)! * binomial(n,k));
for(n=1, 20, print1(a(n),", "))

