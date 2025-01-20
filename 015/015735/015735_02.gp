\\ a(n) = e * (-3)^n * n! * Sum_{k>=0} (-1)^k * binomial(k/3,n)/k!.
a(n) = round( exp(1) * (-3)^n * n! * sum(k=0, 1000, (-1)^k * binomial(k/3, n)/k!) );
for(n=1, 20, print1(a(n), ", "))
