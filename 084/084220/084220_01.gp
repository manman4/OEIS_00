a(n) = sum(i=1, n, sum(j=1, n, sum(k=1, n, ( n/gcd([i, j, k, n]) )^3)));
for(n=1, 50, print1(a(n), ", "))

b(n) = sumdiv(n, d, moebius(n/d) * (n/d)^3 * sigma(d, 6));
for(n=1, 50, print1(a(n)-b(n), ", "))
