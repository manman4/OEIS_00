\\ a(n) = (-1)^(n+1) * 9^(2*n+1) * Sum_{k>=0} (-1/8)^(k+1) * binomial(k/9,n).
a(n) = (-1)^(n+1)*9^(2*n+1)*sum(k=0,n+1000, (-1/8.0)^(k+1) * binomial(k/9, n));
for(n=0, 15, print1(round(a(n)),", "))
for(n=0, 15, print1(a(n),", "))


