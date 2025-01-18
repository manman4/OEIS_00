M=20;

bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a005011(n) = 5^n * bell(n, 1/5);
for(n=0, M, print1(a005011(n), ", "));

\\ a(n) = Sum_{k=0..n} |Stirling1(n,k)| * A005011(k).
a(n) = sum(k=0, n, abs(stirling(n, k, 1)) * a005011(k));
for(n=0, M, print1(a(n), ", "));

\\ a(n) = (1/exp(1/5)) * (-1)^n * n! * Sum_{k>=0} binomial(-5*k,n)/(5^k * k!). 
b(n) = round( (1/exp(1/5)) * (-1)^n * n! * sum(k=0, 1000, binomial(-5*k, n)/(5^k * k!)) );
for(n=0, M, print1(a(n)-b(n), ", "));
