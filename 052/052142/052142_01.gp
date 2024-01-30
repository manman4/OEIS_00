\\ Cf. A082579, A091695.
\\ a(n) = n! * Sum_{k=0..n} 4^(n-k) * binomial(n-k/2-1,n-k)/k!.
a(n) = n! * sum(k=0, n, 4^(n-k) * binomial(n-k/2-1,n-k)/k!);
for(n=0, 15, print1(a(n),", "))   
      