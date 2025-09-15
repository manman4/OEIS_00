\\ a(n) = Sum_{k=0..n} 3^k * (-2)^(n-k) * binomial(n,k) * binomial(n+k+1,n). 
a(n) = sum(k=0, n, 3^k * (-2)^(n-k) * binomial(n,k) * binomial(n+k+1,n));
for(n=0, 20, print1(a(n),", "))  

