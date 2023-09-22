a(n) = if(n==0, 1, sum(k=0, n-3, binomial(n-1,k+2) * a(n-k-3) ));                   
for(n=0, 25, print1(a(n),", "))  

