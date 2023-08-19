\\ a(n+1) = n! * Sum_{k=0..n} (1/2)^(n-k) * (n+1)^(k-1) * binomial(k,n-k)/k!.

a(n) = (n-1)! * sum(k=0, n-1, (1/2)^(n-1-k) * n^(k-1) * binomial(k,n-1-k)/k!);
for(n=1, 17, print1(a(n),", "))   
