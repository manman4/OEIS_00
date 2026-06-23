\\ T(n,k) = Sum_{j=0..floor(n/2)} binomial(n-j,j) * (binomial(n+k-2*j,k-j) - binomial(n+k-2*j,k-j-1)). 
T(n,k) = sum(j=0, n\2, binomial(n-j,j) * (binomial(n+k-2*j,k-j) - binomial(n+k-2*j,k-j-1)));
for(n=0, 15, for(k=0, n, print1(T(n,k),", ")));