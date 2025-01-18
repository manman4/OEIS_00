M=20;

bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a004211(n) = 2^n * bell(n, 1/2);

\\ a(n) = Sum_{k=0..n} |Stirling1(n,k)| * A004211(k) 
a(n) = sum(k=0, n, abs(stirling(n, k, 1)) * a004211(k));
for(n=0, M, print1(a(n), ", "));

\\ a(n) = (1/exp(1/2)) * (-1)^n * n! * Sum_{k>=0} binomial(-2*k,n)/(2^k * k!).
b(n) = round( (1/exp(1/2)) * (-1)^n * n! * sum(k=0, 1000, binomial(-2*k, n)/(2^k * k!)) );
for(n=0, M, print1(a(n)-b(n), ", "));
