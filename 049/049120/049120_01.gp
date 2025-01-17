\\ a(n) = (1/e) * (-4)^n * n! * Sum_{k>=0} binomial(-k/4,n)/k!.
a(n) = round( 1/exp(1) * (-4)^n * n! * sum(k=0, 1000, binomial(-k/4, n)/k!) );
for(n=1, 20, print1(a(n), ", "))
