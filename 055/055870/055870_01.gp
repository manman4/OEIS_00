\\ n行目の係数を求める
\\ Sum_{k=0..n} T(n,k) * x^k = exp( -Sum_{k>=1} Fibonacci(n*k)/Fibonacci(k) * x^k/k ).
v(n) = my(x='x+O('x^(n+10))); Vec( exp( -sum(k=1, n+10, fibonacci(n*k)/fibonacci(k) * x^k/k )) );
for(n=0, 10, print(v(n)));


\\ 0行目は1
a = [1];
for(n=1, 10, a = concat(a, v(n)[1..n+1]));
print(a);