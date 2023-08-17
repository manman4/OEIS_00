default(parisize, 120000000)

a(n, m) = polcoef(1/prod(k=1,n, 1-k*x^k+x*O(x^n))^m, n);
print(a(5000, 24))