\\ G.f.: x*B(x)^2 where B(x) is the g.f. of A045721.
my(N=30, x='x+O('x^N)); Vec(sum(k=0, N, binomial(3*k+1, k)*x^k))
my(N=30, x='x+O('x^N)); Vec(sum(k=0, N, binomial(3*k+1, k)*x^k)^2)