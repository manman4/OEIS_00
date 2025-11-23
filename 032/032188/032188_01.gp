M=20;

\\ G.f.: (x/2) * Sum_{k>=0} Product_{j=0..k-1} ((1/2) * (1 + j*x)).
my(N=15, x='x+O('x^N)); Vec( (x/2) * sum(k=0, 500, prod(j=0, k-1, (1/2)*(1. + j*x)))  )
my(N=15, x='x+O('x^N)); apply(round, Vec( (x/2) * sum(k=0, 500, prod(j=0, k-1, (1/2)*(1. + j*x)))  ))

