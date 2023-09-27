a(n) = 1/n * sum(k=0, (n-1)\3, (-1)^k * binomial(n,k) * binomial(2*n-2-2*k,n-1-3*k) );
for(n=1, 17, print1(a(n),", "))   
