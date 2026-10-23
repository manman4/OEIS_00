my(N=30, x='x+O('x^N)); concat(0, Vec(serlaplace(log(1+x)*cosh(x)))) 

\\ a(n) = (-1)^(n+1) * (A002104(n) + A002741(n))/2.
a002104(n) = sum(k=0, n-1,          binomial(n, k) * (n-k-1)!);
a002741(n) = sum(k=0, n-1, (-1)^k * binomial(n, k) * (n - k - 1)!);
a(n) = (-1)^(n+1) * (a002104(n) + a002741(n))/2;
for(n=0, 18, print1(a(n), ", "));

