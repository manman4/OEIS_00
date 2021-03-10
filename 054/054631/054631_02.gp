T(n, k) = sum(j=1, n, k^gcd(j, n))/n;
for(n=1, 10, for(k=1, n, print1(T(n, k), ", ")))