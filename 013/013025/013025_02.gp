M=24;

a(n) = if(n==0, 1, 2*sum(k=0, (n-1)\4, binomial(n-1, 4*k)*a(n-4*k-1))); 
for(n=0, M, print1(a(n), ", "));