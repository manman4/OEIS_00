\\ n行目の係数を求める
\\ Sum_{k=0..n} T(n,k) * x^k = exp( -Sum_{k>=1} Fibonacci(n*k)/Fibonacci(k) * x^k/k ).
v(n) = my(x='x+O('x^(n+1))); Vec( exp( -sum(k=1,n+3, fibonacci(n*k)/fibonacci(k) * x^k/k )) );
for(n=0, 10, print(v(n)));
