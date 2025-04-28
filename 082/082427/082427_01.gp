\\ E.g.f.: 2 + 3*x/2 + (11*x/2 - 2*exp(x))/(1-x)^2.
my(N=30, x='x+O('x^N)); Vec(serlaplace( 2 + 3*x/2 + (11*x/2 - 2*exp(x))/(1-x)^2 ))
