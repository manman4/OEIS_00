\\ E.g.f.: exp( 1/6 * Sum_{k>=1} binomial(6*k,k) * x^k/k ).
my(N=20, x='x+O('x^N)); Vec(serlaplace(exp( 1/6 * sum(k=1, N, binomial(6*k,k)*x^k/k ))))

