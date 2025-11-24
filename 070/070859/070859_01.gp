\\ a(n) = 3 * (7*n^3+13*n^2+12*n+8) * (2*n)!/(n! * (n+4)!).
a(n) = 3 * (7*n^3 + 13*n^2 + 12*n + 8) * (2*n)!/(n! * (n + 4)!);
for(n=0, 26, print1(a(n),", "));

\\ a(n) = binomial(2*n,n) + 5*binomial(2*n,n-2)/(n+3) - binomial(2*n,n-4).
b(n) = binomial(2*n, n) + 5*binomial(2*n, n-2)/(n+3) - binomial(2*n, n-4);
for(n=0, 100, print1(a(n)-b(n),", "));