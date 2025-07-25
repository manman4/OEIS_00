\\ A131689 Triangle of numbers T(n,k) = k!*Stirling2(n,k)

\\ a(n) = Sum_{k=0..n} A131689(n,k) * 4^k.
a(n) = sum(k=0, n, 4^k * k! * stirling(n, k, 2));
for(n=0, 15, print1(a(n),", "));

\\ a(n) = (4/5) * Sum_{k=0..n} 5^k * (-1)^(n-k) * A131689(n,k) for n > 0. 
b(n) = (4/5) * sum(k=0, n, 5^k * (-1)^(n-k) * k! * stirling(n, k, 2));
for(n=0, 50, print1(a(n)-b(n),", "));




