my(N=100, x='x+O('x^N)); Vec(x / (1 - x)^2 )

my(N=100, x='x+O('x^N)); Vec(sum(k=1, N, eulerphi(k) * x^k / (1 - x^k) ))