M=24;

a(n) = if(n==0, 1, -2*sum(k=0, (n-3)\4, binomial(n-1, 4*k+2)*a(n-4*k-3))); 
for(n=0, M, print1(a(n), ", "));