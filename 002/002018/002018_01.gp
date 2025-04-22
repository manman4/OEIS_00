\\ a(n) = (n-1)! * n! * Sum_{k=0..n-1} (-1)^k * (1/2)^(n-k-1) * binomial(-3/2,k)/(n-k-1)! for n > 0.
a(n) = if(n==0, 1, (n-1)! * n! * sum(k=0, n-1, (-1)^k * (1/2)^(n-k-1) * binomial(-3/2,k)/(n-k-1)!));
for(n=0, 20, print1(a(n),", "))