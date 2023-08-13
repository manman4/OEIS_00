a(n) = if(n==0, 1, -(64/n) * sum(k=1, n, sigma(k)*a(n-k)));  
for(n=0, 15, print1(a(n),", "))                                                 
 