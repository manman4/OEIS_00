my(N=100, x='x+O('x^N)); Vec(sum(k=1, N, moebius(k) * x^k / (1 - x^k) ))

my(N=100, x='x+O('x^N)); sum(k=1, N, moebius(k) * x^k / (1 - x^k) )