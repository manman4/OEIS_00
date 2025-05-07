\\ G.f. of column k: x^k * exp( Sum_{j>=1} Fibonacci((k+1)*j)/Fibonacci(j) * x^j/j ).
T(n, k) = my(x='x+O('x^(n+10))); polcoef(x^k * exp( sum(j=1, n+10, fibonacci((k+1)*j)/fibonacci(j) * x^j/j )), n);
for(n=0, 10, for(k=0, n, print1(T(n, k),", ")));