my(N=70, x='x+O('x^N)); Vec(x^8 * sum(k=0, N, x^(8*k)/prod(j=1, 2*k+1, 1-x^j)))