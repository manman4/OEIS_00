\\ a(n) = e * (-4)^n * n! * Sum_{k>=0} (-1)^k * binomial(k/4,n)/k!.
a(n) = round( exp(1) * (-4)^n * n! * sum(k=0, 1000, (-1)^k * binomial(k/4, n)/k!) );
for(n=1, 20, print1(a(n), ", "))
