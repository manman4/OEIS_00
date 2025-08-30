\\ a(n) = Sum_{k=0..n} (1-2*i)^k * (1+2*i)^(n-k) * binomial(n,k)^2, where i is the imaginary unit.
a(n) = sum(k=0, n, (1-2*I)^k * (1+2*I)^(n-k) * binomial(n,k)^2);
for(n=0, 20, print1(a(n),", "))  
