\\ a(n) = n! * Sum_{k=0..n}(n-k)^k/k!.
a006153(n) = n! * sum(k=0, n, (n-k)^k/k!);

\\ a(n) = A006153(n+1)/(n+1).
a(n) = a006153(n+1)/(n+1);
for(n=0, 15, print1(a(n),", "))  

