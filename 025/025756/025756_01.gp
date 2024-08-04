\\ a(n) = (-1)^(n+1) * 3^(2*n+1) * Sum_{k>=0} (-1/2)^(k+1) * binomial(k/3,n).
a(n) = (-1)^(n+1)*3^(2*n+1)*sum(k=0,n+1000, (-1/2.0)^(k+1) * binomial(k/3, n));
for(n=0, 15, print1(round(a(n)),", "))
for(n=0, 15, print1(a(n),", "))


