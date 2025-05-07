\\ G.f.: x * exp( Sum_{k>=1} F(3*k)/F(k) * x^k/k ), where F(n) = A000045(n).
my(N=30, x='x+O('x^N)); concat(0, Vec( x * exp( sum(k=1, N, fibonacci(3*k)/fibonacci(k) * x^k/k )) ))


