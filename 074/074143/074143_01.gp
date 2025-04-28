\\ E.g.f.: x/2 * (1 + 1/(1-x)^2).
my(N=30, x='x+O('x^N)); Vec(serlaplace( x/2 * (1 + 1/(1-x)^2) ))

