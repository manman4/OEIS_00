M=20;

bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a004212(n) = 3^n * bell(n, 1/3);
for(n=0, M, print1(a004212(n), ", "));

\\ a(n) = Sum_{k=0..n} |Stirling1(n,k)| * A004212(k).
a(n) = sum(k=0, n, abs(stirling(n, k, 1)) * a004212(k));
for(n=0, M, print1(a(n), ", "));

\\ a(n) = (1/exp(1/3)) * (-1)^n * n! * Sum_{k>=0} binomial(-3*k,n)/(3^k * k!).
b(n) = round( (1/exp(1/3)) * (-1)^n * n! * sum(k=0, 1000, binomial(-3*k, n)/(3^k * k!)) );
for(n=0, M, print1(a(n)-b(n), ", "));
