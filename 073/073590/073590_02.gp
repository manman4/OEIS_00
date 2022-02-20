M=20;

a(n) = sum(k=1, n, k!*binomial(n, k)*sum(j=1, k, (-1)^(j+1)/j)); 
for(n=1, M, print1(a(n), ", "));