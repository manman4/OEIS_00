a(n) = (1/n)  * sum(k=0, (n-1)\3, (-1)^k * binomial(n-1+k,n-1) * binomial(2*n-2-3*k,n-1) );                   
for(n=1, 15, print1(a(n),", "))  

