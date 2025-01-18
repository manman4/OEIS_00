bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a004213(n) = 4^n * bell(n, 1/4);
for(n=0, 20, print1(a004213(n), ", "));

\\ a(n) = Sum_{k=0..n} Stirling1(n,k) * A004213(k).
a(n) = sum(k=0, n, stirling(n, k, 1) * a004213(k));
for(n=0, 20, print1(a(n), ", "));

\\ a(n) = (1/exp(1/4)) * n! * Sum_{k>=0} binomial(4*k,n)/(4^k * k!). 
b(n) = round( (1/exp(1/4)) * n! * sum(k=0, 1000, binomial(4*k, n)/(4^k * k!)) );
for(n=0, M, print1(a(n)-b(n), ", "));