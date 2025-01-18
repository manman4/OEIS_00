bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a005012(n) = 6^n * bell(n, 1/6);
for(n=0, 20, print1(a005012(n), ", "));

\\ a(n) = Sum_{k=0..n} Stirling1(n,k) * A005012(k). 
a(n) = sum(k=0, n, stirling(n, k, 1) * a005012(k));
for(n=0, 20, print1(a(n), ", "));

\\ a(n) = (1/exp(1/6)) * n! * Sum_{k>=0} binomial(6*k,n)/(6^k * k!).
b(n) = round( (1/exp(1/6)) * n! * sum(k=0, 1000, binomial(6*k, n)/(6^k * k!)) );
for(n=0, M, print1(a(n)-b(n), ", "));