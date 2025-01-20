\\ a(n) = e * (-5)^n * n! * Sum_{k>=0} (-1)^k * binomial(k/5,n)/k!.
a(n) = round( exp(1) * (-5)^n * n! * sum(k=0, 1000, (-1)^k * binomial(k/5, n)/k!) );
for(n=1, 20, print1(a(n), ", "))
