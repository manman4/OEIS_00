my(N=70, x='x+O('x^N)); Vec(x^6 * sum(k=0, N, x^(12*k)/prod(j=1, 2*k, 1-x^j)))