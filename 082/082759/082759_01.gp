\\ a(n) = Sum_{k=0..n} binomial(n,k) * A385975(k).
a385975(n) = sum(k=0, n, binomial(n, k)*binomial(2*k, n-k));
a(n) = sum(k=0, n, binomial(n, k)*a385975(k));
for(n=0, 25, print1(a(n), ", "))

