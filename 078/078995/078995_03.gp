\\ G.f.: B(x)^2 where B(x) is the g.f. of A005810.
my(N=30, x='x+O('x^N)); Vec(sum(k=0, N, binomial(4*k, k)*x^k))
my(N=30, x='x+O('x^N)); Vec(sum(k=0, N, binomial(4*k, k)*x^k)^2)