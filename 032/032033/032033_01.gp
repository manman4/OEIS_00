a(n) = if(n==0, 1, 3*a(n-1) - 4*sum(k=1, n-1, (-1)^k * binomial(n-1,k) * a(n-k)));   
for(n=0, 15, print1(a(n),", "))     
