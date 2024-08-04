\\ a(n) = (-1)^(n+1) * 7^(2*n+1) * Sum_{k>=0} (-1/6)^(k+1) * binomial(k/7,n). 
a(n) = (-1)^(n+1)*7^(2*n+1)*sum(k=0,n+1000, (-1/6.0)^(k+1) * binomial(k/7, n));
for(n=0, 15, print1(round(a(n)),", "))
for(n=0, 15, print1(a(n),", "))


