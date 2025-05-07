\\ G.f.: exp( Sum_{k>=1} F(4*k)/F(k) * x^k/k ), where F(n) = A000045(n).
my(N=30, x='x+O('x^N)); Vec( exp( sum(k=1, N, fibonacci(4*k)/fibonacci(k) * x^k/k )) )

