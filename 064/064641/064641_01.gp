\\ a(n) = (-1)^n * (n+1) + Sum_{k=0..n-1} (a(k) + (-1)^k) * (a(n-1-k) + (-1)^(n-1-k)).
a(n) = (-1)^n * (n+1) + sum(k=0, n-1, (a(k) + (-1)^k) * (a(n-1-k) + (-1)^(n-1-k)));
for(n=0, 15, print1(a(n),", "));

