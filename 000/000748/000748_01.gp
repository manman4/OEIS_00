\\ a(n) = (-1)^n * Sum_{k=0..floor(n/3)} (-1)^k * binomial(n+2,3*k+2).
a(n) = (-1)^n*sum(k=0,n\3, (-1)^k * binomial(n+2,3*k+2));
for(n=0, 25, print1(a(n),", "))

  