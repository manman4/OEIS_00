\\ a(n) = Sum_{k=0..n} 2^(n-k) * (Product_{j=0..k-1} (2*j+1)) * Stirling1(n,k).
a(n) = sum(k=0, n, 2^(n-k) * (prod(j=0, k-1, 2*j+1)) * stirling(n,k,1));
for(n=0, 15, print1(a(n),", "))   

