\\ Cf. A099927

\\ G.f. of column k: x^k * exp( Sum_{j>=1} Fibonacci((k+1)*j)/Fibonacci(j) * x^j/j ).
T(n, k) = my(x='x+O('x^(n+10))); polcoef(x^k * exp( sum(j=1, n+10, fibonacci((k+1)*j)/fibonacci(j) * x^j/j )), n);
for(n=0, 10, for(k=0, n, print1(T(n, k),", ")));

\\ T(n, k) = Fibonacci(n-k-1)*T(n-1, k-1) + Fibonacci(k+1)*T(n-1, k).
S(n,k) = if(n*k==0, n^k, fibonacci(n-k-1)*S(n-1, k-1) + fibonacci(k+1)*S(n-1, k));
for(n=0, 15, for(k=0, n, print1(T(n, k)-S(n, k),", ")));