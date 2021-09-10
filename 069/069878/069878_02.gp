M=3;

a(n) = polcoef(prod(k=1, 10^n, 1+x^k+x*O(x^(10^n))), 10^n); 
for(n=0, M, print1(a(n), ", "));