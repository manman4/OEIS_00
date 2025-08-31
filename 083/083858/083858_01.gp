\\ a(n) = Sum_{k=0..n-1} 3^k * 2^(n-1-k) * binomial(k,n-1-k). 
a(n) = sum(k=0, n-1, 3^k * 2^(n-1-k) * binomial(k,n-1-k));
for(n=0, 20, print1(a(n),", "));