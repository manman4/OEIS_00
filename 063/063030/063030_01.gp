a(n) = if(n==0, 0, 1/n * sum(k=0, (n-1)\3, binomial(n-1+k,n-1) * binomial(2*n-2-3*k,n-1) ));
for(n=0, 17, print1(a(n),", "))   
