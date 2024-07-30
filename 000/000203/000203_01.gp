my(N=50, x='x+O('x^N)); Vec(sum(k=1, N, sigma(k, 1) * x^k ))

my(N=50, x='x+O('x^N)); Vec(sum(k=1, N, k * x^k/(1 - x^k) ))

my(N=50, x='x+O('x^N)); Vec(sum(k=1, N, x^k/(1 - x^k)^2 ))