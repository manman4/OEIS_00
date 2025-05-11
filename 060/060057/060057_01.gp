a059419(n, k) = n!*polcoef(tan(x+x*O(x^n))^k/k!, n);
for(n=0, 10, for(k=0, n, print1(a059419(n, k),", ")); print);

a(n) = a059419(2*n+5, 5); 
for(n=0, 20, print1(a(n),", "))