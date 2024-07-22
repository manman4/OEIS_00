my(N=100, x='x+O('x^N)); Vec(prod(k=1, N, 1 - x^k ))


my(N=100, x='x+O('x^N)); Vec(sum(k=-N, N, (-1)^k * x^(k*(3*k+1)/2) ))