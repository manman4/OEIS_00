M=14;

a(n) = if(n==0, 1, sum(k=1, 2*n, ( sum(i=0, k-1, (i-k)^(2*n) * binomial(2*k,i) * (-1)^i) )/2^(k-1)) ); 
for(n=0, M, print1(a(n), ", "));