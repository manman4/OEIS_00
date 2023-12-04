a(n) = if(n==0, 1, n * sum(k=2, n, binomial(n-1,k-1) * a(n-k) ));
for(n=0, 20, print1(a(n),", "))  