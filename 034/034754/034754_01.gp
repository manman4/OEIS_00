M=30;

a(n) = sum(k=1, n, 3^(gcd(k, n)-1));
for(n=1, M, print1(a(n), ", "));