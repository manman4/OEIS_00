M=19;

\\ a(n) = (-1)^n * n! * Sum_{k=0..n} 3^k * binomial(-1/3,k)/(n-k)!. 
a(n) = (-1)^n * n! * sum(k=0, n, 3^k * binomial(-1/3,k)/(n-k)!);
for(n=0, M, print1(a(n),", "))
