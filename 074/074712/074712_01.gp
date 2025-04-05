\\ A(n, k) = n + k - gcd(n, k). 
a(n, k) = n + k - gcd(n, k);
for(n=1, 15, for(k=1, n, print1(a(k, n-k+1),", ")))

\\ T(n,k) = A(k,n-k+1) = n+1 - A050873(n+1,k).
T(n, k) = n + 1 - gcd(n + 1, k);
for(n=1, 15, for(k=1, n, print1(T(n, k),", ")))


