\\ a(n) = (-1)^(n+1) * 8^(2*n+1) * Sum_{k>=0} (-1/7)^(k+1) * binomial(k/8,n).
a(n) = (-1)^(n+1)*8^(2*n+1)*sum(k=0,n+1000, (-1/7.0)^(k+1) * binomial(k/8, n));
for(n=0, 15, print1(round(a(n)),", "))
for(n=0, 15, print1(a(n),", "))


