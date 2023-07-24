a(n) = (-1)^n * sum(k=0, n, (-2)^k * binomial(n,k) * binomial(2*k+1,n) / (2*k+1) ); 
for(n=0, 18, print1(a(n),", ")) 