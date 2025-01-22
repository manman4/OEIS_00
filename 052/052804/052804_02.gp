\\ a(0) = a(1) = 0; a(n) = n * (n-2)! + Sum_{k=2..n-1} k * (k-2)! * binomial(n-1,k) * a(n-k).
a(n) = if(n<2, 0, n*(n-2)! + sum(k=2, n-1, k*(k-2)! * binomial(n-1, k) * a(n-k)));  
for(n=0, 20, print1(a(n), ", "))
