T(n, k) = polcoef(x^k*sum(i=0, n, x^(2*k*i)/prod(j=1, 2*i+1, 1-x^j+x*O(x^n))), n);
for(n=1, 14, for(k=1, n, print1(T(n,k),", ")))    