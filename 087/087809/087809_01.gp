default(parisize, 120000000)

\\ a(n) = [x^(3*n+1)] (1-x) * (1+x)^(3*n-1) * (Sum_{k=0..n+1} x^k)^3.
a(n) = polcoef((1-x) * (1+x)^(3*n-1) * (sum(k=0, n+1, x^k))^3, 3*n+1);
for(n=0, 1000, write("b087809_1.txt", n, " ", a(n)));