\\ a(n) = Sum_{k=0..n} 4^k * (-3)^(n-k) * binomial(n,k) * binomial(n+k+1,n). 
a(n) = sum(k=0, n, 4^k * (-3)^(n-k) * binomial(n,k) * binomial(n+k+1,n));
for(n=0, 20, print1(a(n),", "))  

