my(N=50, x='x+O('x^N)); Vec(sum(k=1, N, ramanujantau(k) * x^k ))

my(N=50, x='x+O('x^N)); Vec(x * prod(k=1, N, (1 - x^k)^24 ))