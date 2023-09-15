my(N=10000, q='q+O('q^N)); Vec( -1/3 * sum(k=3, N, (-1)^k * k * binomial(k+2,5) * q^(k^2)) / (1+2*sum(k=1, N, (-q)^(k^2))))

