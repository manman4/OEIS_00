Bo(r,n) = if(n==0, 1, r*Bo(r,n-1) - (r+1)*sum(j=1, n-1, (-1)^j * binomial(n-1,j) * Bo(r,n-j)) );
for(r=1, 9, for(n=1, r, print1(Bo(r-n+1, n),", ")))    

a(r,n) = if(n==0, 1,  r * sum(k=1, n, binomial(n, k)*a(r, n-k) ) );
for(r=1, 15, for(n=1, r, print1(a(r-n+1, n) - Bo(r-n+1, n),", ")))    
