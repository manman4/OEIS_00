my(N=100, x='x+O('x^N)); Vec(prod(k=1, N, (1 - x^k)^3 ))


my(N=100, x='x+O('x^N)); Vec(sum(k=0, N, (-1)^k * (2*k+1) * x^(k*(k+1)/2) ))