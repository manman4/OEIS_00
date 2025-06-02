\\ a(n) = Sum_{k=0..n} Stirling2(n, k) * (3^k) * k!.
a(n) = sum(k=0, n, 3^k * k! * stirling(n, k, 2));
for(n=0, 15, print1(a(n),", "));

\\ a(n) = (3/4) * Sum_{k=0..n} 4^k * (-1)^(n-k) * k! * Stirling2(n,k) for n > 0.
b(n) = (3/4) * sum(k=0, n, 4^k * (-1)^(n-k) * k! * stirling(n, k, 2));
for(n=0, 50, print1(a(n)-b(n),", "));




